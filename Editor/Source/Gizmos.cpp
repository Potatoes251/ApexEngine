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
    m_circle.SetRhi(rhi);
    CreateCube();
    CreateCone();
    CreateCircle();
    CreateCylinder();
    m_cube.UploadToGpu();
    m_cone.UploadToGpu();
    m_cylinder.UploadToGpu();
    m_circle.UploadToGpu();
}

Gizmos::~Gizmos()
{
    //m_cube.SetRhi(nullptr);
    //m_cone.SetRhi(nullptr);
    //m_cylinder.SetRhi(nullptr);
    //m_circle.SetRhi(nullptr);
    //m_shader->SetRhi(nullptr);
}

void Gizmos::DrawGizmos(Data::Object* object, GizmoMode mode, LibMath::Matrix4 viewProj, LibMath::Vector3 viewPos)
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
		DrawTranslation(object);
		break;
	case GizmoMode::Rotation:
		DrawRotation(object);
		break;
	case GizmoMode::Scale:
		DrawScale(object);
		break;
	default:
		assert(false && "Unhandled gizmo mode");
		break;
	}
}

Axis Gizmos::IsOnGizmo(
    LibMath::Vector2 mousePosOnScreen, GizmoMode mode, LibMath::Vector3 worldPos, 
    LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const
{
    switch (mode)
    {
    case GizmoMode::Translation:    return IsOnTranslation(mousePosOnScreen, worldPos, viewProj, panelOffset, panelSize);
    case GizmoMode::Rotation:       return IsOnRotation(mousePosOnScreen, worldPos, viewProj, panelOffset, panelSize);
    case GizmoMode::Scale:          return IsOnScale(mousePosOnScreen, worldPos, viewProj, panelOffset, panelSize);
    default:
        assert(false && "Unhandled gizmo mode");
        break;
    }
    return Axis::None;
}

void Gizmos::DrawTranslation(Data::Object* object)
{
    LibMath::Vector3 pos = object->GetGlobalTransform().getPosition();

    float length = 1.0f * m_distanceToCamera;
    float thickness = 0.05f * m_distanceToCamera;
    constexpr float handleScale = 2.f;

    // X axis (RED)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(length * 0.5f, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 1,0,0 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(length, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", model);
        m_cone.Draw();
    }
    // Y axis (GREEN)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, length * 0.5f, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,1,0 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, length, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", model);
        m_cone.Draw();
    }
    // Z axis (BLUE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, 0, length * 0.5f),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,0,1 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, 0, length),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", model);
        m_cone.Draw();
    }
}

void Gizmos::DrawRotation(Data::Object* object)
{
    LibMath::Vector3 pos = object->GetGlobalTransform().getPosition();

    // X axis (RED)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos,
            { 0,0,1 }, LibMath::Degree(90),
            LibMath::Vector3( 1, 1, 1 ) * m_distanceToCamera);

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 1,0,0 });
        m_circle.DrawLineLoop();
    }
    // Y axis (GREEN)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos,
            { 0,1,0 }, LibMath::Degree(0),
            LibMath::Vector3(1, 1, 1) * m_distanceToCamera);

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,1,0 });
        m_circle.DrawLineLoop();
    }
    // Z axis (BLUE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos,
            { 1,0,0 }, LibMath::Degree(-90),
            LibMath::Vector3(1, 1, 1) * m_distanceToCamera);

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,0,1 });
        m_circle.DrawLineLoop();
    }
}

void Gizmos::DrawScale(Data::Object* object)
{
    LibMath::Vector3 pos = object->GetGlobalTransform().getPosition();

    float length = 1.0f * m_distanceToCamera;
    float thickness = 0.05f * m_distanceToCamera;
    constexpr float handleScale = 2.f;

    // X axis (RED)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(length * 0.5f, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 1,0,0 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(length, 0, 0),
            { 0,0,1 }, LibMath::Degree(-90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", model);
        m_cube.Draw();
    }
    // Y axis (GREEN)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, length * 0.5f, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,1,0 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, length, 0),
            { 1,0,0 }, LibMath::Degree(0),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", model);
        m_cube.Draw();
    }
    // Z axis (BLUE)
    {
        LibMath::Matrix4 model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, 0, length * 0.5f),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,0,1 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, 0, length),
            { 1,0,0 }, LibMath::Degree(90),
            { thickness * handleScale, thickness * handleScale, thickness * handleScale });

        m_shader->SetUniform("uModel", model);
        m_cube.Draw();
    }
}

Axis Gizmos::IsOnTranslation(
    LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, 
    LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const
{
    float axisLength = 1.0f * m_distanceToCamera;
    float mouseRadius = 15.0f * m_distanceToCamera;

    LibMath::Circle mouse(LibMath::Point2D(mousePosOnScreen[0], mousePosOnScreen[1]), mouseRadius);
    LibMath::Vector2 screenCenter = GetScreenPos(worldPos, viewProj, panelOffset, panelSize);

    const LibMath::Vector3 axes[3] = {
        {axisLength, 0, 0},
        {0, axisLength, 0},
        {0, 0, axisLength}
    };

    for (int i = 0; i < 3; ++i)
    {
        LibMath::Vector2 endPos = GetScreenPos(worldPos + axes[i], viewProj, panelOffset, panelSize);
        LibMath::Line2D line(LibMath::Point2D(screenCenter[0], screenCenter[1]),
            LibMath::Point2D(endPos[0], endPos[1]));

        if (LibMath::colisionLineCircle(line, mouse))
        {
            return static_cast<Axis>(i);
        }
    }

    return Axis::None;
}

Axis Gizmos::IsOnRotation(
    LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, 
    LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const
{
    float axisLength = 1.0f * m_distanceToCamera;

    float tolerance = 0.15f * axisLength;

    LibMath::Line3D ray = ScreenToWorldRay(mousePosOnScreen, viewProj, panelOffset, panelSize);

    LibMath::Vector3 hit;

    const LibMath::Vector3 axes[3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    LibMath::Vector2 screenCenter = GetScreenPos(worldPos, viewProj, panelOffset, panelSize);
    LibMath::Circle mouseCircle(LibMath::Point2D(mousePosOnScreen[0], mousePosOnScreen[1]), 15.0f * m_distanceToCamera);

    for (int i = 0; i < 3; ++i)
    {
        LibMath::Plane plane(axes[i], -worldPos[i]);

        // circle is almost a line
        if (std::fabs((ray.getDirection().dot(axes[i]))) < 0.05f)
        {
            LibMath::Vector3 tangent;

            if (i == 0) tangent = { 0,1,0 };
            if (i == 1) tangent = { 1,0,0 };
            if (i == 2) tangent = { 1,0,0 };
            // Use simple 2D line-circle collision instead
            LibMath::Vector2 startPos = GetScreenPos(worldPos - tangent * axisLength, viewProj, panelOffset, panelSize);
            LibMath::Vector2 endPos = GetScreenPos(worldPos + tangent * axisLength, viewProj, panelOffset, panelSize);
            LibMath::Line2D axisLine(LibMath::Point2D(startPos[0], startPos[1]), LibMath::Point2D(endPos[0], endPos[1]));

            if (LibMath::colisionLineCircle(axisLine, mouseCircle))
                return static_cast<Axis>(i);
            continue;
        }

        if (LibMath::intersectionRayPlane(ray, plane, hit))
        {
            if (std::fabs(hit.distanceFrom(worldPos) - axisLength) < tolerance)
            {
                return static_cast<Axis>(i);
            }
        }
    }

    return Axis::None;
}

Axis Gizmos::IsOnScale(
    LibMath::Vector2 mousePosOnScreen, LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, 
    LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize) const
{
    float axisLength = 1.0f * m_distanceToCamera;
    float mouseRadius = 15.0f * m_distanceToCamera;

    LibMath::Circle mouse(LibMath::Point2D(mousePosOnScreen[0], mousePosOnScreen[1]), mouseRadius);
    LibMath::Vector2 screenCenter = GetScreenPos(worldPos, viewProj, panelOffset, panelSize);

    const LibMath::Vector3 axes[3] = {
        {axisLength, 0, 0},
        {0, axisLength, 0},
        {0, 0, axisLength}
    };

    for (int i = 0; i < 3; ++i)
    {
        LibMath::Vector2 endPos = GetScreenPos(worldPos + axes[i], viewProj, panelOffset, panelSize);
        LibMath::Line2D line(LibMath::Point2D(screenCenter[0], screenCenter[1]),
            LibMath::Point2D(endPos[0], endPos[1]));

        if (LibMath::colisionLineCircle(line, mouse))
        {
            return static_cast<Axis>(i);
        }
    }

    return Axis::None;
}

void Gizmos::CreateCube()
{
    m_cube.m_vertices =
    {
        // Front (+Z)
        {{-0.5f,-0.5f, 0.5f},{0,0,1},{0,0}},{{0.5f,-0.5f, 0.5f},{0,0,1},{1,0}},
        {{ 0.5f, 0.5f, 0.5f},{0,0,1},{1,1}},{{-0.5f, 0.5f, 0.5f},{0,0,1},{0,1}},
        // Back (-Z)
        {{ 0.5f,-0.5f,-0.5f},{0,0,-1},{0,0}},{{-0.5f,-0.5f,-0.5f},{0,0,-1},{1,0}},
        {{-0.5f, 0.5f,-0.5f},{0,0,-1},{1,1}},{{ 0.5f, 0.5f,-0.5f},{0,0,-1},{0,1}},
        // Left (-X)
        {{-0.5f,-0.5f,-0.5f},{-1,0,0},{0,0}},{{-0.5f,-0.5f, 0.5f},{-1,0,0},{1,0}},
        {{-0.5f, 0.5f, 0.5f},{-1,0,0},{1,1}},{{-0.5f, 0.5f,-0.5f},{-1,0,0},{0,1}},
        // Right (+X)
        {{ 0.5f,-0.5f, 0.5f},{1,0,0},{0,0}},{{ 0.5f,-0.5f,-0.5f},{1,0,0},{1,0}},
        {{ 0.5f, 0.5f,-0.5f},{1,0,0},{1,1}},{{ 0.5f, 0.5f, 0.5f},{1,0,0},{0,1}},
        // Top (+Y)
        {{-0.5f, 0.5f, 0.5f},{0,1,0},{0,0}},{{ 0.5f, 0.5f, 0.5f},{0,1,0},{1,0}},
        {{ 0.5f, 0.5f,-0.5f},{0,1,0},{1,1}},{{-0.5f, 0.5f,-0.5f},{0,1,0},{0,1}},
        // Bottom (-Y)
        {{-0.5f,-0.5f,-0.5f},{0,-1,0},{0,0}},{{ 0.5f,-0.5f,-0.5f},{0,-1,0},{1,0}},
        {{ 0.5f,-0.5f, 0.5f},{0,-1,0},{1,1}},{{-0.5f,-0.5f, 0.5f},{0,-1,0},{0,1}},
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
    m_cone.m_vertices.push_back({ {0, height * 0.5f, 0}, {0,0,0}, {0,0} });
    unsigned int tipIndex = 0;

    // Base center
    m_cone.m_vertices.push_back({ {0, -height * 0.5f, 0}, {0,0,0}, {0,0} });
    unsigned int baseCenterIndex = 1;

    // Base ring vertices
    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * g_twoPi;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        m_cone.m_vertices.push_back({ {x, -height * 0.5f, z}, {0,0,0}, {0,0} });
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
    m_cylinder.m_vertices.push_back({ {0,  halfHeight, 0}, {0,0,0}, {0,0} });
    m_cylinder.m_vertices.push_back({ {0, -halfHeight, 0}, {0,0,0}, {0,0} });

    // Rings
    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * g_twoPi;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // top vertex
        m_cylinder.m_vertices.push_back({ {x,  halfHeight, z}, {0,0,0}, {0,0} });
        // bottom vertex
        m_cylinder.m_vertices.push_back({ {x, -halfHeight, z}, {0,0,0}, {0,0} });
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

void Gizmos::CreateCircle()
{
    constexpr float radius = 1.0f;
    constexpr int segments = 32;

    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * g_twoPi;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        m_circle.m_vertices.push_back({ {x, 0, z}, {0,0,0}, {0,0} });

        // connect current to next
        m_circle.m_indices.push_back(i);
        m_circle.m_indices.push_back((i + 1) % segments);
    }
}


LibMath::Vector2 Apex::Editor::GetScreenPos(
    LibMath::Vector3 worldPos, LibMath::Matrix4 const& viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize)
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
    LibMath::Vector2 mousePosOnScreen, LibMath::Matrix4 viewProj, LibMath::Vector2 panelOffset, LibMath::Vector2 panelSize)
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
