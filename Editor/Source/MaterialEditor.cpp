#include "MaterialEditor.h"

#include "Shader.h"

#include <sstream>

using namespace Apex::Editor;
using namespace Apex::Rendering;

static constexpr float LABEL_W = 160.f;
static constexpr float PREVIEW_S = 96.f;

void MaterialEditor::Open(std::string path)
{
    m_open = true;
    m_wasReady = false;
    m_currentMat = m_resourceManager->CreateAsync<Material>(path);
}

void MaterialEditor::Close()
{
    m_open = false;
    m_wasReady = false;
    m_currentMat = {};
}

void MaterialEditor::Draw()
{
	if (!m_open) return;

	m_gui->BeginPanel("MaterialEditor", &m_open, false, true);

    if (!m_currentMat.IsReady())
    {
        m_gui->Text("Loading...");
        m_gui->EndPanel();
        return;
    }
    // first frame ready
    if (!m_wasReady && m_currentMat->GetComputeShader().IsValid())
    {
        m_computeShader = m_currentMat->GetComputeShader()->GetPath();
    }

    m_wasReady = true;

    if (!m_open)
    {
        Close();
        m_gui->EndPanel();
        return;
    }

    DrawPreviewAndHeader();

    m_gui->Separator();

    DrawTint();
    DrawTextureList();
    m_gui->Separator();
    DrawShaderPicker(m_computeShader, m_compShadPickOpen, "Compute");

    m_texturePicker.Draw();
    m_gui->EndPanel();
}

void Apex::Editor::MaterialEditor::SetThumbnailRenderer(ThumbnailRenderer* t)
{
    m_thumbnails = t;
}

void Apex::Editor::MaterialEditor::DrawPreviewAndHeader()
{
    // Left: sphere preview thumbnail
    if (m_thumbnails)
    {
        uint32_t thumb = m_thumbnails->GetOrRenderMaterial(m_currentMat->GetPath());
        if (thumb != 0)
        {
            m_gui->DrawImageTinted(thumb, { PREVIEW_S, PREVIEW_S }, { 1,1,1,1 }, true);
            m_gui->SameLine(0.f, 10.f);
        }
    }

    // Right: name + save button
    m_gui->BeginChildPanel("##mat_header", 0.f, PREVIEW_S, false);

    m_gui->PushFont(FontID::Large);
    m_gui->Text(std::filesystem::path(m_currentMat->GetPath()).stem().string());
    m_gui->PopFont();

    m_gui->TextDisabled(m_currentMat->GetPath());

    float width = m_gui->GetAvailableSize()[0];
    if (m_gui->ButtonSized("Save", { width, 26.f }))
        SaveMaterial();

    if (m_gui->ButtonSized("Add Texture", { width, 26.f }))
        m_currentMat->AddEmptyTexture();

    m_gui->EndChildPanel();
}

void Apex::Editor::MaterialEditor::DrawTint()
{
    m_gui->AlignTextToFramePadding();
    m_gui->Text("Tint:");
    m_gui->SameLine(LABEL_W);
    m_gui->SetNextItemWidth(-1.f);

    LibMath::Vector3 tint = m_currentMat->GetTint();
    float col[3] = { tint[0], tint[1], tint[2] };
    if (m_gui->ColorEdit3("##tint", col))
        m_currentMat->SetTint({ col[0], col[1], col[2] });
}

void Apex::Editor::MaterialEditor::DrawTextureList()
{
    auto& textures = m_currentMat->GetTextures();
    for (int i = 0; i < static_cast<int>(textures.size()); ++i)
        DrawTexture(&textures[i], i);
}

void Apex::Editor::MaterialEditor::SaveMaterial()
{
    if (!m_computeShader.empty())
    {
        m_currentMat->SetComputeShader(m_resourceManager->CreateAsync<ComputeShader>(m_computeShader));
    }
    m_currentMat->Save();
}

void MaterialEditor::DrawTexture(Resources::ResourceHandle<Texture>* texture, int id)
{
    std::string label = "Texture " + std::to_string(id) + ":";
    m_gui->AlignTextToFramePadding();
    m_gui->Text(label);
    m_gui->SameLine(LABEL_W);

    uint32_t textureID = texture->IsValid() ? EditorIcon::Get((*texture)->GetPath()) : 0;
    if (textureID != 0)
    {
        m_gui->DrawImageTinted(textureID, { 22.f, 22.f }, { 1,1,1,1 }, true);
        m_gui->SameLine(0.f, 4.f);
    }

    m_gui->SetNextItemWidth(-70.f);
    m_gui->TextDisabled(texture->IsValid() ? std::filesystem::path((*texture)->GetPath()).filename().string() : "None");

    m_gui->Separator();
    float width = m_gui->GetAvailableSize()[0];
    if (m_gui->ButtonSized(std::string("Select##tex") + std::to_string(id), { width, 22.f }))
        m_texturePicker.Open(texture);
}

void MaterialEditor::DrawShaderPicker(std::string& out_shaderPath, bool& out_open, const std::string shaderType)
{
    m_gui->AlignTextToFramePadding();
    m_gui->Text(shaderType + " Shader:");
    m_gui->SameLine(LABEL_W);
    m_gui->SetNextItemWidth(-80.f);
    m_gui->TextDisabled(out_shaderPath.empty() ? "None" : out_shaderPath);

    m_gui->Separator();
    if (m_gui->ButtonSized("Select##shad" + shaderType, { m_gui->GetAvailableSize()[0], 22.f }))
        out_open = true;

    if (!out_open) return;

    m_gui->BeginModalBlock("Select " + shaderType + " Shader");
    m_gui->BeginPanel("Select " + shaderType + " Shader", &out_open, false, true);

    if (!std::filesystem::exists("ApexAssets/") || !out_open)
    {
        m_gui->EndPanel();
        m_gui->EndModalBlock();
        out_open = false;
        return;
    }

    for (auto& entry : std::filesystem::recursive_directory_iterator("ApexAssets/"))
    {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension();
        if (ext != ".glsl" && ext != ".frag" && ext != ".vert") continue;

        std::string path = entry.path().generic_string();
        if (m_gui->Button(path))
        {
            out_shaderPath = path;
            out_open = false;
            break;
        }
    }

    m_gui->EndPanel();
    m_gui->EndModalBlock();
}
