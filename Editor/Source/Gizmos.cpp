#include "Gizmos.h"

#include "Object.h"

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
    m_cube.SetRhi(nullptr);
    m_cone.SetRhi(nullptr);
    m_cylinder.SetRhi(nullptr);
    m_circle.SetRhi(nullptr);
    m_shader->SetRhi(nullptr);
}

void Gizmos::DrawGizmos(Data::Object* object, GizmoMode mode, LibMath::Matrix4 viewProj)
{
    //if (!object) return;

    if (!m_shader.IsReady()) return;

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

bool Gizmos::IsOnGizmo(LibMath::Vector2 pointOnScreen)
{
    return false;
}

void Gizmos::DrawTranslation(Data::Object* object)
{
    //LibMath::Vector3 pos = object->GetGlobalTransform().getPosition();
    LibMath::Vector3 pos;
    float length = 1.0f;
    float thickness = 0.05f;

    // X axis (RED)
    {
        // cylinder
        auto model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(length * 0.5f, 0, 0),
            { 0,0,1 },
            LibMath::Degree(-90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 1,0,0 });
        m_cylinder.Draw();

        // cone
        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(length, 0, 0),
            { 0,0,1 },
            LibMath::Degree(-90),
            { thickness * 2, thickness * 2, thickness * 2 });

        m_shader->SetUniform("uModel", model);
        m_cone.Draw();
    }

    // Y axis (GREEN)
    {
        auto model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, length * 0.5f, 0),
            { 1,0,0 },
            LibMath::Degree(0),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,1,0 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, length, 0),
            { 1,0,0 },
            LibMath::Degree(0),
            { thickness * 2, thickness * 2, thickness * 2 });

        m_shader->SetUniform("uModel", model);
        m_cone.Draw();
    }

    // Z axis (BLUE)
    {
        auto model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, 0, length * 0.5f),
            { 1,0,0 },
            LibMath::Degree(90),
            { thickness, length, thickness });

        m_shader->SetUniform("uModel", model);
        m_shader->SetUniform("uColor", { 0,0,1 });
        m_cylinder.Draw();

        model = LibMath::Matrix4::createTransform(
            pos + LibMath::Vector3(0, 0, length),
            { 1,0,0 },
            LibMath::Degree(90),
            { thickness * 2, thickness * 2, thickness * 2 });

        m_shader->SetUniform("uModel", model);
        m_cone.Draw();
    }
}

void Gizmos::DrawRotation(Data::Object* object)
{
}

void Gizmos::DrawScale(Data::Object* object)
{
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

    // ----- Side triangles -----
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

    // ----- Top cap -----
    for (int i = 0; i < segments; i++)
    {
        unsigned int current = 2 + i * 2;
        unsigned int next = 2 + ((i + 1) % segments) * 2;

        m_cylinder.m_indices.push_back(topCenter);
        m_cylinder.m_indices.push_back(next);
        m_cylinder.m_indices.push_back(current);
    }

    // ----- Bottom cap -----
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
