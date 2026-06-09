#include "MeshViewer.h"
#include "EditorIcon.h"

#include "Mesh.h"
#include "Model.h"

#include "LibMath/Trigonometry.h"
#include "LibMath/Arithmetic.h"

#include <glad/glad.h>

#include <sstream>
#include <iomanip>

using namespace Apex::Rendering;
using namespace Apex::UserInterface;
using namespace Apex::Editor;


// Constructor / Destructor

MeshViewer::MeshViewer(IGUI* gui,
    IRHI* rhi,
    Apex::Resources::ResourceManager* resourceManager)
    : m_gui(gui), m_rhi(rhi), m_resourceManager(resourceManager)
{
    BuildShaders();
}

MeshViewer::~MeshViewer()
{
    UnloadMesh();
    DestroyShaders();

    m_rhi->DeleteFrameBuffer(m_fbo);
}

// Public API
void MeshViewer::Open(const Fs::path& path, int instanceIndex)
{
    UnloadMesh();
    m_path = path;
    m_filename = path.filename().string();
    m_instanceIndex = instanceIndex;
    m_fileBytes = Fs::exists(path) ? Fs::file_size(path) : 0;
    m_open = true;
    m_pendingFocus = false;
    m_displayMode = MeshDisplayMode::Lit;
    m_showGrid = true;
    m_showBounds = false;
    m_showPivot = true;
    m_showNormals = false;
    m_exposure = 1.f;
    m_lightDir = { 0.577f, 0.577f, 0.577f };
    LoadMesh(path);
}

void MeshViewer::Close()
{
    UnloadMesh();
    m_open = false;
}

void MeshViewer::Focus()
{
    m_pendingFocus = true;
}

// Draw 
void MeshViewer::Draw()
{
    if (!m_open) return;

    m_gui->PushFont(FontID::Default);

    constexpr float CASCADE = 30.f;
    float offset = static_cast<float>(m_instanceIndex) * CASCADE;
    LibMath::Vector2 screen = m_gui->GetScreenSize();
    m_gui->SetNextWindowPos(LibMath::Vector2(offset, offset));
    m_gui->SetNextWindowSize(LibMath::Vector2(screen[0] - offset, screen[1] - offset));

    if (m_pendingFocus)
    {
        m_gui->SetNextWindowFocus();
        m_pendingFocus = false;
    }

    m_gui->BeginPanel("Mesh Viewer - " + m_filename, &m_open, false, true);

    if (!m_open)
    {
        Close();
        m_gui->EndPanel();
        m_gui->PopFont();
        return;
    }

    // Finalize stats/bounds/camera once the async load completes
    if (!m_statsValid && m_model.IsReady())
        FinalizeMeshLoad();

    DrawToolbar();
    m_gui->Separator();

    float totalHeight = m_gui->GetAvailableSize()[1];

    m_gui->BeginChildPanel("##mv_viewport", -DETAILS_WIDTH, totalHeight, false);
    DrawViewport();
    m_gui->EndChildPanel();

    m_gui->SameLine();

    m_gui->BeginChildPanel("##mv_details", DETAILS_WIDTH, totalHeight, true);
    DrawDetails();
    m_gui->EndChildPanel();

    m_gui->EndPanel();
    m_gui->PopFont();
}

// Toolbar 
void MeshViewer::DrawToolbar()
{
    m_gui->PushStyleVariable(StyleVariable::FramePadding, 6.f, 4.f);

    DrawToolbarDisplayMode();

    m_gui->SameLine(0.f, 16.f);
    m_gui->VerticalSeparator();
    m_gui->SameLine(0.f, 16.f);

    DrawToolbarShowFlags();

    m_gui->SameLine(0.f, 16.f);
    m_gui->VerticalSeparator();
    m_gui->SameLine(0.f, 16.f);

    DrawToolbarLighting();

    m_gui->SameLine(0.f, 16.f);
    m_gui->VerticalSeparator();
    m_gui->SameLine(0.f, 16.f);

    if (m_gui->Button("Fit")) ResetCamera();

    m_gui->PopStyleVariable();
}

void MeshViewer::DrawToolbarDisplayMode()
{
    // Lit
    {
        bool active = (m_displayMode == MeshDisplayMode::Lit);
        if (active) m_gui->PushColor(StyleColor::Button, Color{ 0.2f,0.5f,0.9f,1.f });
        if (m_gui->Button("Lit")) m_displayMode = MeshDisplayMode::Lit;
        if (active) m_gui->PopColor();
    }
    // Unlit
    m_gui->SameLine(0.f, 4.f);
    {
        bool active = (m_displayMode == MeshDisplayMode::Unlit);
        if (active) m_gui->PushColor(StyleColor::Button, Color{ 0.2f,0.5f,0.9f,1.f });
        if (m_gui->Button("Unlit")) m_displayMode = MeshDisplayMode::Unlit;
        if (active) m_gui->PopColor();
    }
    // Wireframe
    m_gui->SameLine(0.f, 4.f);
    {
        bool active = (m_displayMode == MeshDisplayMode::Wireframe);
        if (active) m_gui->PushColor(StyleColor::Button, Color{ 0.2f,0.5f,0.9f,1.f });
        if (m_gui->Button("Wireframe")) m_displayMode = MeshDisplayMode::Wireframe;
        if (active) m_gui->PopColor();
    }
    // Normals
    m_gui->SameLine(0.f, 4.f);
    {
        bool active = (m_displayMode == MeshDisplayMode::Normals);
        if (active) m_gui->PushColor(StyleColor::Button, Color{ 0.2f,0.5f,0.9f,1.f });
        if (m_gui->Button("Normals"))   m_displayMode = MeshDisplayMode::Normals;
        if (active) m_gui->PopColor();
    }
}

void MeshViewer::DrawToolbarShowFlags()
{
    auto toggleButton = [&](const char* label, bool& flag)
        {
            bool wasActive = flag;
            if (wasActive) m_gui->PushColor(StyleColor::Button, Color{ 0.25f,0.55f,0.25f,1.f });
            if (m_gui->Button(label)) flag = !flag;
            if (wasActive) m_gui->PopColor();
            m_gui->SameLine(0.f, 4.f);
        };

    toggleButton("Grid", m_showGrid);
    toggleButton("Bounds", m_showBounds);
    toggleButton("Pivot", m_showPivot);
    toggleButton("Normal Lines", m_showNormals);
}

void MeshViewer::DrawToolbarLighting()
{
    m_gui->AlignTextToFramePadding();
    m_gui->Text("EV:");
    m_gui->SameLine(0.f, 6.f);
    m_gui->SetNextItemWidth(70.f);
    m_gui->SliderFloat("##mv_ev", &m_exposure, 0.f, 4.f);
}

// Viewport 
void MeshViewer::DrawViewport()
{
    LibMath::Vector2 panelPos = m_gui->GetPanelPos();
    LibMath::Vector2 panelSize = m_gui->GetAvailableSize();

    // Rebuild FBO if the panel resized
    if (panelSize[0] != m_fboWidth || panelSize[1] != m_fboHeight)
        RebuildFBO(panelSize[0], panelSize[1]);

    // Render scene into FBO
    if (m_fbo != 0 && m_shadersReady)
        RenderScene();

    // Draw FBO color texture as background of this child panel
    if (m_fboColor != 0)
        m_gui->DrawImageBackground(
            m_rhi->GetTexture(m_fboColor),
            panelPos,
            LibMath::Vector2(panelPos[0] + panelSize[0], panelPos[1] + panelSize[1]));

    // Invisible button for input
    LibMath::Vector2 cursor = m_gui->GetCursorPos();
    m_gui->SetCursorPos(panelPos);
    m_gui->InvisibleButton("##mv_interact", panelSize);
    bool hovered = m_gui->IsItemHovered();
    bool active = m_gui->IsItemActive();
    m_gui->SetCursorPos(cursor);

    HandleViewportInput(panelPos, panelSize, hovered, active);

    // Overlay: zoom label bottom-left
    {
        IDrawList* drawList = m_gui->GetDrawList();
        std::string zoomLabel = std::to_string(static_cast<int>(100.f * 3.f / m_orbitDistance)) + "%";
        constexpr float FONT_SIZE = 16.f, PAD = 5.f;
        LibMath::Vector2 size = drawList->CalculateTextSizeEx(zoomLabel, FONT_SIZE);
        LibMath::Vector2 pos(panelPos[0] + 8.f,panelPos[1] + panelSize[1] - size[1] - PAD * 2.f - 8.f);

        drawList->DrawRectFilled(
            LibMath::Vector2(pos[0] - PAD, pos[1] - PAD),
            LibMath::Vector2(pos[0] + size[0] + PAD, pos[1] + size[1] + PAD),
            0xBB000000, 4.f);
        drawList->DrawTextEx(pos, 0xFFFFFFFF, zoomLabel, FONT_SIZE);
    }
}

void MeshViewer::HandleViewportInput(LibMath::Vector2 /*panelPos*/,
    LibMath::Vector2 /*panelSize*/,
    bool hovered, bool active)
{
    if (!hovered && !active) return;

    LibMath::Vector2 delta = m_gui->GetMouseDelta();

    // Left drag - orbit
    if (active && m_gui->IsMouseDown(MouseButton::Left))
    {
        m_orbitYaw += delta[0] * 0.4f;
        m_orbitPitch -= delta[1] * 0.4f;
        m_orbitPitch = std::max(-89.f, std::min(89.f, m_orbitPitch));
    }

    // Scroll - zoom
    if (hovered)
    {
        float wheel = m_gui->GetMouseWheelDelta();
        if (wheel != 0.f)
        {
            m_orbitDistance *= (wheel > 0.f ? 0.9f : 1.1f);
            m_orbitDistance = std::max(0.05f, std::min(500.f, m_orbitDistance));
        }
    }

    // F5 - reset camera
    if (hovered && m_gui->IsKeyPressed(UIKey::F5))
        ResetCamera();
}

void MeshViewer::ResetCamera()
{
    // Frame the bounding box
    LibMath::Vector3 center = 
    {
        (m_boundsMin[0] + m_boundsMax[0]) * 0.5f,
        (m_boundsMin[1] + m_boundsMax[1]) * 0.5f,
        (m_boundsMin[2] + m_boundsMax[2]) * 0.5f 
    };

    float distanceX = m_boundsMax[0] - m_boundsMin[0];
    float distanceY = m_boundsMax[1] - m_boundsMin[1];
    float distanceZ = m_boundsMax[2] - m_boundsMin[2];
    float radius = std::sqrt(distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ) * 0.5f;
    if (radius < 1e-4f) radius = 1.f;

    m_orbitTarget = center;
    m_orbitDistance = radius * 2.5f;
    m_orbitYaw = 45.f;
    m_orbitPitch = 25.f;
}

// Details 
void MeshViewer::DrawDetails()
{
    m_gui->PushStyleVariable(StyleVariable::ItemSpacing, 4.f, 6.f);

    m_gui->PushFont(FontID::Large);
    m_gui->Text("Details");
    m_gui->PopFont();
    m_gui->Separator();

    DrawDetailsMeshSection();
    DrawDetailsBoundsSection();
    DrawDetailsViewSection();

    m_gui->PopStyleVariable();
}

void MeshViewer::DrawDetailsMeshSection()
{
    if (!m_gui->CollapsingHeader("Mesh", true)) return;

    auto row = [&](const char* label, const std::string& value)
        {
            m_gui->AlignTextToFramePadding();
            m_gui->Text(label);
            m_gui->SameLine(LABEL_COL_WIDTH);
            m_gui->TextDisabled(value);
        };

    row("File:", m_filename);
    row("Size:", FileSizeString());
    row("Vertices:", std::to_string(m_vertexCount));
    row("Triangles:", std::to_string(m_triangleCount));
}

void MeshViewer::DrawDetailsBoundsSection()
{
    if (!m_gui->CollapsingHeader("Bounding Box", true)) return;

    auto formatV3 = [](LibMath::Vector3 v) -> std::string
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "(%.3f, %.3f, %.3f)", v[0], v[1], v[2]);
            return buffer;
        };

    LibMath::Vector3 size = 
    {
        m_boundsMax[0] - m_boundsMin[0],
        m_boundsMax[1] - m_boundsMin[1],
        m_boundsMax[2] - m_boundsMin[2] 
    };

    auto row = [&](const char* label, const std::string& value)
        {
            m_gui->AlignTextToFramePadding();
            m_gui->Text(label);
            m_gui->SameLine(LABEL_COL_WIDTH);
            m_gui->TextDisabled(value);
        };

    row("Min:", formatV3(m_boundsMin));
    row("Max:", formatV3(m_boundsMax));
    row("Size:", formatV3(size));
}

void MeshViewer::DrawDetailsViewSection()
{
    if (!m_gui->CollapsingHeader("View", true)) return;

    m_gui->AlignTextToFramePadding();
    m_gui->Text("Exposure:");
    m_gui->SameLine(LABEL_COL_WIDTH);
    m_gui->SetNextItemWidth(-1.f);
    m_gui->SliderFloat("##det_ev", &m_exposure, 0.f, 4.f);

    m_gui->AlignTextToFramePadding();
    m_gui->Text("Normal length:");
    m_gui->SameLine(LABEL_COL_WIDTH);
    m_gui->SetNextItemWidth(-1.f);
    m_gui->SliderFloat("##det_nlen", &m_normalLength, 0.001f, 0.5f);
}

// 3-D rendering 
void MeshViewer::RenderScene()
{
    if (!m_model.IsReady() || m_model->GetMeshes().size() == 0) return;

    UpdateOrbitCamera();

    m_rhi->BindFrameBuffer(m_fbo, m_fboWidth, m_fboHeight);

    m_rhi->SetCullingFace(false);
    m_rhi->SetClearColor(0.12f, 0.12f, 0.14f, 1.f);
    m_rhi->Clear();

    LibMath::Matrix4 matrixViewProj = GetViewProj();
    LibMath::Matrix4 model = LibMath::Matrix4::identity(); // identity
    LibMath::Vector3 camPos = m_camera.GetPosition();

    Apex::Rendering::RHIShaderHandle shader = 0;
    switch (m_displayMode)
    {
    case MeshDisplayMode::Lit:
        shader = m_shaderLit;
        m_rhi->BindShader(shader);
        m_rhi->SetUniformMat4(shader, "uMVP", matrixViewProj);
        m_rhi->SetUniformMat4(shader, "uModel", model);
        m_rhi->SetUniformVec3(shader, "uLightDir", m_lightDir);
        m_rhi->SetUniformVec3(shader, "uCamPos", camPos);
        m_rhi->SetUniformFloat(shader, "uExposure", m_exposure);
        m_rhi->SetUniformVec3(shader, "uBaseColor", { 0.75f,0.75f,0.75f });
        break;

    case MeshDisplayMode::Unlit:
        shader = m_shaderUnlit;
        m_rhi->BindShader(shader);
        m_rhi->SetUniformMat4(shader, "uMVP", matrixViewProj);
        m_rhi->SetUniformMat4(shader, "uModel", model);
        m_rhi->SetUniformVec3(shader, "uBaseColor", { 0.75f,0.75f,0.75f });
        m_rhi->SetUniformFloat(shader, "uExposure", m_exposure);
        break;

    case MeshDisplayMode::Wireframe:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        shader = m_shaderWireframe;
        m_rhi->BindShader(shader);
        m_rhi->SetUniformMat4(shader, "uMVP", matrixViewProj);
        m_rhi->SetUniformFloat(shader, "uColor_r", 0.8f);
        m_rhi->SetUniformFloat(shader, "uColor_g", 0.8f);
        m_rhi->SetUniformFloat(shader, "uColor_b", 0.8f);
        m_rhi->SetUniformFloat(shader, "uColor_a", 1.0f);
        break;

    case MeshDisplayMode::Normals:
        shader = m_shaderNormals;
        m_rhi->BindShader(shader);
        m_rhi->SetUniformMat4(shader, "uMVP", matrixViewProj);
        m_rhi->SetUniformMat4(shader, "uModel", model);
        break;
    }

    if (shader != 0)
    {
        for (Mesh const& mesh : m_model->GetMeshes())
            mesh.Draw();
    }

    if (m_displayMode == MeshDisplayMode::Wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Normals overlay on top of shaded mesh
    if (m_showNormals && m_displayMode != MeshDisplayMode::Wireframe)
        RenderNormalsOverlay();

    if (m_showGrid)   RenderGrid();
    if (m_showBounds) RenderBoundingBox();
    if (m_showPivot)  RenderPivot();

    m_rhi->UnBindFrameBuffer();
}

void MeshViewer::RenderGrid()
{
    if (m_shaderFlat == 0) return;

    LibMath::Matrix4 matrixViewProj = GetViewProj();
    m_rhi->BindShader(m_shaderFlat);
    m_rhi->SetUniformMat4(m_shaderFlat, "uMVP", matrixViewProj);

    constexpr int   HALF = 10;
    constexpr float STEP = 1.f;
    constexpr float ALPHA = 0.4f;

    glLineWidth(1.f);
    glEnable(GL_BLEND);

    GLuint tmpVAO, tmpVBO;
    glGenVertexArrays(1, &tmpVAO);
    glGenBuffers(1, &tmpVBO);

    std::vector<float> verts;
    verts.reserve((HALF * 2 + 1) * 4 * 3);

    auto push = [&](float x, float y, float z) 
        { 
            verts.push_back(x); 
            verts.push_back(y); 
            verts.push_back(z); 
        };

    for (int i = -HALF; i <= HALF; ++i)
    {
        float f = static_cast<float>(i) * STEP;
        // Lines along Z
        push(f, 0.f, -HALF * STEP);
        push(f, 0.f, HALF * STEP);
        // Lines along X
        push(-HALF * STEP, 0.f, f);
        push(HALF * STEP, 0.f, f);
    }

    glBindVertexArray(tmpVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tmpVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_r", 0.45f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_g", 0.45f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_b", 0.45f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_a", ALPHA);

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 3));

    glBindVertexArray(0);
    glDeleteBuffers(1, &tmpVBO);
    glDeleteVertexArrays(1, &tmpVAO);
}

void MeshViewer::RenderBoundingBox()
{
    if (m_shaderFlat == 0) return;

    LibMath::Vector3& mn = m_boundsMin;
    LibMath::Vector3& mx = m_boundsMax;

    // 8 corners of the AABB
    float corners[8][3] = 
    {
        {mn[0],mn[1],mn[2]}, {mx[0],mn[1],mn[2]},
        {mx[0],mx[1],mn[2]}, {mn[0],mx[1],mn[2]},
        {mn[0],mn[1],mx[2]}, {mx[0],mn[1],mx[2]},
        {mx[0],mx[1],mx[2]}, {mn[0],mx[1],mx[2]},
    };
    // 12 edges
    unsigned int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0}, // bottom face
        {4,5},{5,6},{6,7},{7,4}, // top face
        {0,4},{1,5},{2,6},{3,7}  // verticals
    };

    std::vector<float> verts;
    for (auto& e : edges)
    {
        for (int k : {0, 1})
        {
            verts.push_back(corners[e[k]][0]);
            verts.push_back(corners[e[k]][1]);
            verts.push_back(corners[e[k]][2]);
        }
    }

    LibMath::Matrix4 matrixViewProj = GetViewProj();
    m_rhi->BindShader(m_shaderFlat);
    m_rhi->SetUniformMat4(m_shaderFlat, "uMVP", matrixViewProj);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_r", 1.f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_g", 0.85f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_b", 0.0f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_a", 1.0f);

    GLuint tempVAO, tempVBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);
    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 3));
    glBindVertexArray(0);
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

void MeshViewer::RenderPivot()
{
    if (m_shaderFlat == 0) return;

    float cx = (m_boundsMin[0] + m_boundsMax[0]) * 0.5f;
    float cy = (m_boundsMin[1] + m_boundsMax[1]) * 0.5f;
    float cz = (m_boundsMin[2] + m_boundsMax[2]) * 0.5f;

    float dx = m_boundsMax[0] - m_boundsMin[0];
    float dy = m_boundsMax[1] - m_boundsMin[1];
    float dz = m_boundsMax[2] - m_boundsMin[2];
    float pivotScale = std::max({ dx, dy, dz }) * 0.2f;
    if (pivotScale < 1e-4f) pivotScale = 0.1f;

    float axisVerts[6][3] = 
    {
        {cx,   cy, cz}, {cx + pivotScale, cy,   cz},
        {cx,   cy, cz}, {cx,   cy + pivotScale, cz},
        {cx,   cy, cz}, {cx,   cy,   cz + pivotScale},
    };
    float colors[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };

    LibMath::Matrix4 matrixViewProj = GetViewProj();
    m_rhi->BindShader(m_shaderFlat);
    m_rhi->SetUniformMat4(m_shaderFlat, "uMVP", matrixViewProj);

    m_rhi->SetDepthTest(false);

    GLuint tempVAO, tempVBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);
    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVerts), axisVerts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glLineWidth(2.f);

    for (int axis = 0; axis < 3; ++axis)
    {
        m_rhi->SetUniformFloat(m_shaderFlat, "uColor_r", colors[axis][0]);
        m_rhi->SetUniformFloat(m_shaderFlat, "uColor_g", colors[axis][1]);
        m_rhi->SetUniformFloat(m_shaderFlat, "uColor_b", colors[axis][2]);
        m_rhi->SetUniformFloat(m_shaderFlat, "uColor_a", 1.0f);
        glDrawArrays(GL_LINES, axis * 2, 2);
    }

    glBindVertexArray(0);
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
    glLineWidth(1.f);
    m_rhi->SetDepthTest(true);
}

void MeshViewer::RenderNormalsOverlay()
{
    // Visualize normals as lines extending from each vertex
    if (!m_model.IsReady()) return;
    if (m_shaderFlat == 0) return;

    std::vector<float> lines;

    for (Mesh const& mesh : m_model->GetMeshes())
    {
        std::vector<Vertex> const& verts = mesh.m_vertices;
        if (verts.empty()) return;

        for (const auto& v : verts)
        {
            lines.push_back(v.m_position[0]);
            lines.push_back(v.m_position[1]);
            lines.push_back(v.m_position[2]);
            lines.push_back(v.m_position[0] + v.m_normal[0] * m_normalLength);
            lines.push_back(v.m_position[1] + v.m_normal[1] * m_normalLength);
            lines.push_back(v.m_position[2] + v.m_normal[2] * m_normalLength);
        }
    }

    LibMath::Matrix4 matrixViewProj = GetViewProj();
    m_rhi->BindShader(m_shaderFlat);
    m_rhi->SetUniformMat4(m_shaderFlat, "uMVP", matrixViewProj);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_r", 0.0f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_g", 0.7f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_b", 1.0f);
    m_rhi->SetUniformFloat(m_shaderFlat, "uColor_a", 1.0f);

    GLuint tempVAO, tempVBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);
    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(lines.size() * sizeof(float)), lines.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() / 3));
    glBindVertexArray(0);
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

// Setup / teardown 
void MeshViewer::LoadMesh(const Fs::path& path)
{
    m_model = m_resourceManager->CreateAsync<Model>(path.generic_string());
    m_statsValid = false;
}

void MeshViewer::UnloadMesh()
{
    m_model = {};
    m_vertexCount = 0;
    m_triangleCount = 0;
    m_boundsMin = { 1e9f,  1e9f,  1e9f };
    m_boundsMax = { -1e9f, -1e9f, -1e9f };
}

void MeshViewer::FinalizeMeshLoad()
{
    for (Mesh const& mesh : m_model->GetMeshes())
    {
        m_vertexCount += mesh.GetVertexCount();
        m_triangleCount += mesh.GetIndexCount();
    }
    m_triangleCount /= 3;
    m_statsValid = true;
    ComputeBounds();
    ResetCamera();
}

void MeshViewer::ComputeBounds()
{
    m_boundsMin = m_model->GetBoundsMin();
    m_boundsMax = m_model->GetBoundsMax();
}

void MeshViewer::RebuildFBO(float width, float height)
{
    if (width < 1.f || height < 1.f) return;

    m_rhi->DeleteFrameBuffer(m_fbo);

    m_fboWidth = width;
    m_fboHeight = height;
    m_fbo = m_rhi->CreateFrameBuffer(width, height, m_fboColor);
}

void MeshViewer::BuildShaders()
{
    static const std::string VERT = "ApexAssets/Shaders/MeshViewerLitVert.glsl";
    static const std::string VERT_SIMPLE = "ApexAssets/Shaders/MeshViewerSimpleVert.glsl";
    static const std::string FRAG_LIT = "ApexAssets/Shaders/MeshViewerLitFrag.glsl";
    static const std::string FRAG_UNLIT = "ApexAssets/Shaders/MeshViewerUnlitFrag.glsl";
    static const std::string FRAG_WIRE = "ApexAssets/Shaders/MeshViewerFlatFrag.glsl";
    static const std::string FRAG_NORMALS = "ApexAssets/Shaders/MeshViewerNormalsFrag.glsl";

    m_shaderLit = m_rhi->CreateShader({ VERT, FRAG_LIT });
    m_shaderUnlit = m_rhi->CreateShader({ VERT, FRAG_UNLIT });
    m_shaderWireframe = m_rhi->CreateShader({ VERT_SIMPLE, FRAG_WIRE });
    m_shaderNormals = m_rhi->CreateShader({ VERT, FRAG_NORMALS });
    m_shaderFlat = m_rhi->CreateShader({ VERT_SIMPLE, FRAG_WIRE });

    m_shadersReady = (m_shaderLit && m_shaderUnlit && m_shaderWireframe && m_shaderNormals && m_shaderFlat);
}

void MeshViewer::DestroyShaders()
{
    if (m_shaderLit) 
    { 
        m_rhi->DeleteShader(m_shaderLit);       
        m_shaderLit = 0; 
    }
    if (m_shaderUnlit) 
    { 
        m_rhi->DeleteShader(m_shaderUnlit);     
        m_shaderUnlit = 0; 
    }
    if (m_shaderWireframe) 
    { 
        m_rhi->DeleteShader(m_shaderWireframe); 
        m_shaderWireframe = 0; 
    }
    if (m_shaderNormals) 
    { 
        m_rhi->DeleteShader(m_shaderNormals);   
        m_shaderNormals = 0; 
    }
    if (m_shaderFlat) 
    { 
        m_rhi->DeleteShader(m_shaderFlat);      
        m_shaderFlat = 0; 
    }
    m_shadersReady = false;
}

// Camera 
void MeshViewer::UpdateOrbitCamera()
{
    // Keep m_camera position in sync for the uCamPos lighting uniform
    LibMath::Radian yaw = LibMath::Degree(m_orbitYaw);
    LibMath::Radian pitch = LibMath::Degree(m_orbitPitch);
    float x = m_orbitTarget[0] + m_orbitDistance * LibMath::cos(pitch) * LibMath::sin(yaw);
    float y = m_orbitTarget[1] + m_orbitDistance * LibMath::sin(pitch);
    float z = m_orbitTarget[2] + m_orbitDistance * LibMath::cos(pitch) * LibMath::cos(yaw);
    m_camera.SetPosition({ x, y, z });
}

LibMath::Vector3 MeshViewer::GetCameraPosition() const
{
    return m_camera.GetPosition();
}

LibMath::Matrix4 MeshViewer::GetViewProj()
{
    // Compute camera position from spherical coordinates
    LibMath::Radian yaw = LibMath::Degree(m_orbitYaw);
    LibMath::Radian pitch = LibMath::Degree(m_orbitPitch);
    float x = m_orbitTarget[0] + m_orbitDistance * LibMath::cos(pitch) * LibMath::sin(yaw);
    float y = m_orbitTarget[1] + m_orbitDistance * LibMath::sin(pitch);
    float z = m_orbitTarget[2] + m_orbitDistance * LibMath::cos(pitch) * LibMath::cos(yaw);

    LibMath::Vector3 eye = { x, y, z };
    LibMath::Vector3 target = m_orbitTarget;
    LibMath::Vector3 worldUp = { 0.f, 1.f, 0.f };

    // Use LibMath lookAt directly -- avoids any yaw/pitch convention mismatch
    LibMath::Matrix4 view = LibMath::Matrix4::lookAt(eye, target, worldUp);

    float aspect = (m_fboHeight > 0.f) ? m_fboWidth / m_fboHeight : 1.f;
    float nearP = m_orbitDistance * 0.001f;
    float farP = m_orbitDistance * 100.f;
    m_camera.SetProjectionMatrix(aspect, nearP, farP);

    return m_camera.GetProjection() * view;
}

// Misc
std::string MeshViewer::FileSizeString() const
{
    if (m_fileBytes >= 1'048'576) // 1'048'576 = 1024 * 1024
        return std::to_string(m_fileBytes / (1'048'576)) + " MB";
    if (m_fileBytes >= 1024)
        return std::to_string(m_fileBytes / 1024) + " KB";
    return std::to_string(m_fileBytes) + " B";
}