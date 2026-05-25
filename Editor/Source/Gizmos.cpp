#include "Gizmos.h"

#include "Object.h"

#include "LibMath/Colision.h"
#include "LibMath/Colision3D.h"

#include <cassert>

using namespace Apex::Editor;
using namespace Apex::Resources;
using namespace Apex::Data;
using namespace Apex::Rendering;

Gizmos::Gizmos(Resources::ResourceHandle<Shader> shader, Rendering::IRHI* rhi) : m_shader(shader)
{
    m_cube.SetRhi(rhi);
    m_cone.SetRhi(rhi);
    m_cylinder.SetRhi(rhi);
    m_thorus.SetRhi(rhi);

    CreateCube();
    CreateCone();
    CreateThorus();
    CreateCylinder();

    m_cube.UploadToGpu();
    m_cone.UploadToGpu();
    m_cylinder.UploadToGpu();
    m_thorus.UploadToGpu();

    m_cube.ClearVertices();
    m_cone.ClearVertices();
    m_cylinder.ClearVertices();
    m_thorus.ClearVertices();
}

void Gizmos::DrawGizmos(const Data::Object* object, GizmoMode mode, LibMath::Matrix4 viewProj, LibMath::Vector3 viewPos, bool picking, bool local)
{
    if (!object || !m_shader.IsReady()) return;

    m_distanceToCamera = viewPos.distanceFrom(object->GetGlobalTransform().getPosition());
    // adjust the value to have a correct size
    m_distanceToCamera *= .1f;

    m_shader->Use();
    m_shader->SetUniform("uViewProj", viewProj);

	switch (mode)
	{
	case GizmoMode::Translation:
		DrawTranslation(object, picking, local);
		break;
	case GizmoMode::Rotation:
		DrawRotation(object, picking, local);
		break;
	case GizmoMode::Scale:
		DrawScale(object, picking);
		break;
	default:
		assert(false && "Unhandled gizmo mode");
		break;
	}
}

void Gizmos::DrawTranslation(const Data::Object* object, bool picking, bool local)
{
    LibMath::Transform trans = object->GetGlobalTransform();
    trans.setScale(LibMath::Vector3(1.f));

    if (!local) trans.setRotation(LibMath::Radian(0.f), LibMath::Radian(0.f), LibMath::Radian(0.f));

    LibMath::Matrix4 transMatrix = trans;

    DrawTranslationPlanes(transMatrix, picking);
    DrawTranslationCenter(transMatrix, picking);
    DrawTranslationArrows(transMatrix, picking);
}

void Gizmos::DrawTranslationPlanes(LibMath::Matrix4 transMatrix, bool picking)
{
    float planeSize = 0.25f * m_distanceToCamera;
    float planeOffset = planeSize * 0.5f;
    float planeThickness = 0.02f * m_distanceToCamera;
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(planeOffset, planeOffset, 0),
            { 0,0,1 }, LibMath::Degree(0),
            { planeSize, planeSize, planeThickness });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::XY, picking));
        m_cube.Draw();
    }
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(planeOffset, 0, planeOffset),
            { 0,0,1 }, LibMath::Degree(0),
            { planeSize, planeThickness, planeSize });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::XZ, picking));
        m_cube.Draw();
    }
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, planeOffset, planeOffset),
            { 0,0,1 }, LibMath::Degree(0),
            { planeThickness, planeSize, planeSize });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::YZ, picking));
        m_cube.Draw();
    }
}

void Gizmos::DrawTranslationCenter(LibMath::Matrix4 transMatrix, bool picking)
{
    float thickness = 0.05f * m_distanceToCamera;
    constexpr float handleScale = 4.f;

    // (WHITE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, 0, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::XYZ, picking));
        m_cube.Draw();
    }
}

void Gizmos::DrawTranslationArrows(LibMath::Matrix4 transMatrix, bool picking)
{
    float length = 1.0f * m_distanceToCamera;
    float thickness = 0.05f * m_distanceToCamera;
    constexpr float handleScale = 2.f;

    // X axis (RED)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(length * 0.5f, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::X, picking));
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(length, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_cone.Draw();
    }
    // Y axis (GREEN)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, length * 0.5f, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::Y, picking));
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, length, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_cone.Draw();
    }
    // Z axis (BLUE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, 0, length * 0.5f),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::Z, picking));
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, 0, length),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transMatrix * model);
        m_cone.Draw();
    }
}

void Gizmos::DrawRotation(const Data::Object* object, bool picking, bool local)
{
    LibMath::Transform trans = object->GetGlobalTransform();
    trans.setScale(LibMath::Vector3(1.f));

    if (!local) trans.setRotation(LibMath::Radian(0.f), LibMath::Radian(0.f), LibMath::Radian(0.f));
    LibMath::Matrix4 transMatrix = trans;

    // X axis (RED)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0.f),
            { 0,0,1 }, LibMath::Degree(90),
            LibMath::Vector3( 1, 1, 1 ) * m_distanceToCamera);

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::X, picking));

        m_thorus.Draw();
    }
    // Y axis (GREEN)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0.f),
            { 0,1,0 }, LibMath::Degree(0),
            LibMath::Vector3(1, 1, 1) * m_distanceToCamera);

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::Y, picking));

        m_thorus.Draw();
    }
    // Z axis (BLUE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0.f),
            { 1,0,0 }, LibMath::Degree(-90),
            LibMath::Vector3(1, 1, 1) * m_distanceToCamera);

        m_shader->SetUniform("uModel", transMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::Z, picking));

        m_thorus.Draw();
    }
}

void Gizmos::DrawScale(const Data::Object* object, bool picking)
{
    LibMath::Vector3 pos = object->GetGlobalTransform().getPosition();

    LibMath::Transform transform = object->GetGlobalTransform();
    transform.setScale(LibMath::Vector3(1.f));
    LibMath::Matrix4 transformMatrix = transform;

    float length = 1.0f * m_distanceToCamera;
    float thickness = 0.05f * m_distanceToCamera;
    constexpr float handleScale = 2.f;

    // central part (WHITE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, 0, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::XYZ, picking));
        m_cube.Draw();
    }
    // X axis (RED)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(length * 0.5f, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::X, picking));
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(length, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_cube.Draw();
    }
    // Y axis (GREEN)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, length * 0.5f, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::Y, picking));
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, length, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_cube.Draw();
    }
    // Z axis (BLUE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, 0, length * 0.5f),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_shader->SetUniform("uColor", GetColor(Axis::Z, picking));
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            LibMath::Vector3(0, 0, length),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", transformMatrix * model);
        m_cube.Draw();
    }
}

LibMath::Vector3 Gizmos::GetColor(Axis axis, bool picking)
{
    if (picking)
    {
        int id = static_cast<int>(axis);
        LibMath::Vector3 color;
        color[0] = (float)((id >> 16) & 0xFF); // highest byte
        color[1] = (float)((id >> 8) & 0xFF);  // middle byte
        color[2] = (float)(id & 0xFF);         // lowest byte
        return color / 255.f;
    }

    switch (axis)
    {
    case Apex::Editor::Axis::X:     return LibMath::Vector3(1.0f, 0.0f, 0.0f);
    case Apex::Editor::Axis::Y:     return LibMath::Vector3(0.0f, 1.0f, 0.0f);
    case Apex::Editor::Axis::Z:     return LibMath::Vector3(0.0f, 0.0f, 1.0f);
    case Apex::Editor::Axis::XY:    return LibMath::Vector3(1.0f, 1.0f, 0.0f);
    case Apex::Editor::Axis::XZ:    return LibMath::Vector3(1.0f, 0.0f, 1.0f);
    case Apex::Editor::Axis::YZ:    return LibMath::Vector3(0.0f, 1.0f, 1.0f);
    case Apex::Editor::Axis::XYZ:   return LibMath::Vector3(1.0f, 1.0f, 1.0f);

    case Apex::Editor::Axis::None:
    default:
        assert(false && "Unhandled axis type");
        break;
    }

    return { 0.f, 0.f, 0.f };
}

void Gizmos::CreateCube()
{
    m_cube.m_vertices =
    {
        // Front (+Z)
        {{-0.5f,-0.5f, 0.5f, 0}},{{0.5f,-0.5f, 0.5f,0}},
        {{ 0.5f, 0.5f, 0.5f, 0}},{{-0.5f, 0.5f, 0.5f,0}},
        // Back (-Z)    
        {{ 0.5f,-0.5f,-0.5f, 0}},{{-0.5f,-0.5f,-0.5f,0}},
        {{-0.5f, 0.5f,-0.5f, 0}},{{ 0.5f, 0.5f,-0.5f,0}},
        // Left (-X)
        {{-0.5f,-0.5f,-0.5f, 0}},{{-0.5f,-0.5f, 0.5f,0}},
        {{-0.5f, 0.5f, 0.5f, 0}},{{-0.5f, 0.5f,-0.5f,0}},
        // Right (+X)    
        {{ 0.5f,-0.5f, 0.5f, 0}},{{ 0.5f,-0.5f,-0.5f,0}},
        {{ 0.5f, 0.5f,-0.5f, 0}},{{ 0.5f, 0.5f, 0.5f,0}},
        // Top (+Y)  
        {{-0.5f, 0.5f, 0.5f, 0}},{{ 0.5f, 0.5f, 0.5f,0}},
        {{ 0.5f, 0.5f,-0.5f, 0}},{{-0.5f, 0.5f,-0.5f,0}},
        // Bottom (-Y)
        {{-0.5f,-0.5f,-0.5f, 0}},{{ 0.5f,-0.5f,-0.5f,0}},
        {{ 0.5f,-0.5f, 0.5f, 0}},{{-0.5f,-0.5f, 0.5f,0}},
    };

    m_cube.m_indices =
    {
         0, 1, 2,  2, 3, 0,   // Front
         4, 5, 6,  6, 7, 4,   // Back
         8, 9,10, 10,11, 8,   // Left
        12,13,14, 14,15,12,   // Right
        16,17,18, 18,19,16,   // Top
        20,21,22, 22,23,20,   // Bottom
    };
}

void Gizmos::CreateCone()
{
    constexpr int segments = 16;
    constexpr float radius = 0.5f;
    constexpr float height = 1.0f;

    // Tip vertex
    m_cone.m_vertices.push_back({ {0, height * 0.5f, 0, 0}, {0,0,0, 0} });
    unsigned int tipIndex = 0;

    // Base center
    m_cone.m_vertices.push_back({ {0, -height * 0.5f, 0, 0}, {0,0,0,0} });
    unsigned int baseCenterIndex = 1;

    // Base ring vertices
    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * g_twoPi;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        m_cone.m_vertices.push_back({ {x, -height * 0.5f, z, 0}, {0,0,0,0} });
    }

    // Side triangles
    for (int i = 0; i < segments; i++)
    {
        unsigned int current = 2 + i;
        unsigned int next = 2 + (i + 1) % segments;

        m_cone.m_indices.push_back(tipIndex);
        m_cone.m_indices.push_back(current);
        m_cone.m_indices.push_back(next);
    }

    // Bottom triangles
    for (int i = 0; i < segments; i++)
    {
        unsigned int current = 2 + i;
        unsigned int next = 2 + (i + 1) % segments;

        m_cone.m_indices.push_back(baseCenterIndex);
        m_cone.m_indices.push_back(next);
        m_cone.m_indices.push_back(current);
    }
}

void Gizmos::CreateCylinder()
{
    constexpr int segments = 16;
    constexpr float radius = 0.5f;
    constexpr float halfHeight = .5f;

    // Top and bottom center vertices
    constexpr unsigned int topCenter = 0;
    constexpr unsigned int bottomCenter = 1;
    m_cylinder.m_vertices.push_back({ {0,  halfHeight, 0, 0}, {0,0,0, 0}});
    m_cylinder.m_vertices.push_back({ {0, -halfHeight, 0, 0}, {0,0,0, 0}});

    // Rings
    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * g_twoPi;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // top vertex
        m_cylinder.m_vertices.push_back({ {x,  halfHeight, z,0}, {0,0,0,0} });
        // bottom vertex
        m_cylinder.m_vertices.push_back({ {x, -halfHeight, z,0}, {0,0,0,0} });
    }

    // Side triangles
    for (int i = 0; i < segments; i++)
    {
        unsigned int top0 = 2 + i * 2;
        unsigned int bot0 = top0 + 1;

        unsigned int top1 = 2 + ((i + 1) % segments) * 2;
        unsigned int bot1 = top1 + 1;

        m_cylinder.m_indices.push_back(top0);
        m_cylinder.m_indices.push_back(bot0);
        m_cylinder.m_indices.push_back(top1);

        m_cylinder.m_indices.push_back(top1);
        m_cylinder.m_indices.push_back(bot0);
        m_cylinder.m_indices.push_back(bot1);
    }
    // Top cap
    for (int i = 0; i < segments; i++)
    {
        unsigned int current = 2 + i * 2;
        unsigned int next = 2 + ((i + 1) % segments) * 2;

        m_cylinder.m_indices.push_back(topCenter);
        m_cylinder.m_indices.push_back(next);
        m_cylinder.m_indices.push_back(current);
    }
    // Bottom cap
    for (int i = 0; i < segments; i++)
    {
        unsigned int current = 3 + i * 2;
        unsigned int next = 3 + ((i + 1) % segments) * 2;

        m_cylinder.m_indices.push_back(bottomCenter);
        m_cylinder.m_indices.push_back(current);
        m_cylinder.m_indices.push_back(next);
    }
}

void Gizmos::CreateThorus()
{
    constexpr float circleRadius = 1.0f;
    constexpr float thickness = 0.03f;

    constexpr int ringSegment = 64;
    constexpr int tubeSegment = 16;

    for (int i = 0; i <= ringSegment; ++i)
    {
        float u = (float)i / ringSegment * 2.0f * g_Pi;

        for (int j = 0; j <= tubeSegment; ++j)
        {
            float v = (float)j / tubeSegment * 2.0f * g_Pi;

            float cosU = cos(u);
            float sinU = sin(u);
            float cosV = cos(v);
            float sinV = sin(v);

            float x = (circleRadius + thickness * cosV) * cosU;
            float y = thickness * sinV;
            float z = (circleRadius + thickness * cosV) * sinU;

            LibMath::Vector4 pos(x, y, z, 0);

            m_thorus.m_vertices.push_back({ pos });
        }
    }

    for (int i = 0; i < ringSegment; ++i)
    {
        for (int j = 0; j < tubeSegment; ++j)
        {
            int a = i * (tubeSegment + 1) + j;
            int b = (i + 1) * (tubeSegment + 1) + j;
            int c = (i + 1) * (tubeSegment + 1) + (j + 1);
            int d = i * (tubeSegment + 1) + (j + 1);

            m_thorus.m_indices.push_back(a);
            m_thorus.m_indices.push_back(b);
            m_thorus.m_indices.push_back(d);

            m_thorus.m_indices.push_back(b);
            m_thorus.m_indices.push_back(c);
            m_thorus.m_indices.push_back(d);
        }
    }
}


LibMath::Vector2 Apex::Editor::GetScreenPos(
    LibMath::Vector3 const& worldPos, LibMath::Matrix4 const& viewProj, 
    LibMath::Vector2 const& panelOffset, LibMath::Vector2 const& panelSize)
{
    LibMath::Vector4 clip = viewProj * LibMath::Vector4(worldPos, 1.0f);

    if (clip[3] <= 0.00001f)
        return LibMath::Vector2(-10000.f);

    LibMath::Vector3 ndc;
    ndc[0] = clip[0] / clip[3];
    ndc[1] = clip[1] / clip[3];
    ndc[2] = clip[2] / clip[3];

    LibMath::Vector2 screen;
    screen[0] = (ndc[0] * 0.5f + 0.5f) * panelSize[0];
    screen[1] = (1.0f - (ndc[1] * 0.5f + 0.5f)) * panelSize[1];

    return screen + panelOffset;
}

LibMath::Line3D Apex::Editor::ScreenToWorldRay(
    LibMath::Vector2 const& mousePosOnScreen, LibMath::Matrix4 const& viewProj, 
    LibMath::Vector2 const& panelOffset, LibMath::Vector2 const& panelSize)
{
    // 1. Adjust mouse for viewport
    float mx = mousePosOnScreen[0] - panelOffset[0];
    float my = mousePosOnScreen[1] - panelOffset[1];

    // 2. NDC
    float x_ndc = (mx / panelSize[0]) * 2.0f - 1.0f;
    float y_ndc = 1.0f - (my / panelSize[1]) * 2.0f;

    // 3. Clip-space near/far
    LibMath::Vector4 nearNDC(x_ndc, y_ndc, -1.0f, 1.0f);
    LibMath::Vector4 farNDC(x_ndc, y_ndc, 1.0f, 1.0f);

    // 4. Transform to world
    LibMath::Matrix4 invVP = viewProj;
    invVP.inverse();

    LibMath::Vector4 nearWorld = invVP * nearNDC;
    LibMath::Vector4 farWorld = invVP * farNDC;
    nearWorld /= nearWorld[3];
    farWorld /= farWorld[3];

    // 5. Build ray
    LibMath::Point3D origin(nearWorld[0], nearWorld[1], nearWorld[2]);
    LibMath::Vector3 dir = (LibMath::Vector3(farWorld) - LibMath::Vector3(nearWorld)).normalized();

    return LibMath::Line3D(origin, dir);
}
