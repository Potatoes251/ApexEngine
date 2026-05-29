#include "DebugRenderer.h"

#include <glad/glad.h>

using namespace Apex::Rendering;
using namespace Apex::Resources;


void DebugRenderer::Initialize(Apex::Resources::ResourceHandle<Shader> shader, IRHI* rhi)
{
	assert(!m_instance && "DebugRenderer is already initialized!");
	m_instance.reset(new DebugRenderer(shader, rhi));
}

DebugRenderer& DebugRenderer::Get()
{
	assert(m_instance && "DebugRenderer not initialized!");
	return *m_instance;
}

void DebugRenderer::AddBox(LibMath::Vector3 pos, LibMath::Quaternion rotation, LibMath::Vector3 halfExtents)
{
	float x = halfExtents[0];
	float y = halfExtents[1];
	float z = halfExtents[2];
	LibMath::Vector3 corners[8] =
	{
		{x ,-y ,-z }, {x ,-y ,z }, {x ,y ,-z }, {x ,y ,z },
		{-x ,-y, -z }, {-x ,-y ,z }, {-x ,y ,-z }, {-x ,y ,z },
	};

	// rotate the points
	for (LibMath::Vector3& corner : corners)
	{
		corner = pos + (rotation.rotate(corner));
	}

	// +X face
	AddLine(corners[0], corners[1]);
	AddLine(corners[1], corners[3]);
	AddLine(corners[3], corners[2]);
	AddLine(corners[2], corners[0]);

	// -X face
	AddLine(corners[4], corners[5]);
	AddLine(corners[5], corners[7]);
	AddLine(corners[7], corners[6]);
	AddLine(corners[6], corners[4]);

	// Connections
	AddLine(corners[0], corners[4]);
	AddLine(corners[1], corners[5]);
	AddLine(corners[2], corners[6]);
	AddLine(corners[3], corners[7]);
}

void DebugRenderer::AddCapsule(LibMath::Vector3 pos, LibMath::Quaternion rotation, float radius, float halfHeight)
{
	constexpr int	segments = 10;
	constexpr int   rings = segments / 2;
	constexpr float fullStep = g_twoPi / (float)segments;

	auto transformPoint =
		[&](LibMath::Vector3 const& localPoint)
		{
			return pos + (rotation.rotate(localPoint));
		};

	LibMath::Vector3 topLocal = { 0.f,  halfHeight, 0.f };
	LibMath::Vector3 bottomLocal = { 0.f, -halfHeight, 0.f };

	// Top & Bottom circles
	for (int i = 0; i < segments; ++i)
	{
		float a0 = i * fullStep;
		float a1 = (i + 1) * fullStep;

		LibMath::Vector3 p0 = { cos(a0) * radius, 0.f, sin(a0) * radius };
		LibMath::Vector3 p1 = { cos(a1) * radius, 0.f, sin(a1) * radius };

		AddLine(transformPoint(topLocal + p0), transformPoint(topLocal + p1));

		AddLine(transformPoint(bottomLocal + p0), transformPoint(bottomLocal + p1));
	}

	// 4 vertical lines
	LibMath::Vector3 sideOffsets[4] =
	{
		{ radius, 0.f, 0.f },
		{-radius, 0.f, 0.f },
		{ 0.f, 0.f,  radius },
		{ 0.f, 0.f, -radius }
	};

	for (int i = 0; i < 4; ++i)
	{
		AddLine(transformPoint(topLocal + sideOffsets[i]), transformPoint(bottomLocal + sideOffsets[i]));
	}

	// Hemispheres
	for (int r = 0; r < rings; ++r)
	{
		float v0 = (static_cast<float>(r) / rings) * (g_Pi / 2.0f);
		float v1 = (static_cast<float>(r + 1) / rings) * (g_Pi / 2.0f);

		for (int i = 0; i < 4; ++i)
		{
			float a = i * g_twoPi / 4;

			LibMath::Vector3 h0 = { cos(a) * cos(v0) * radius, sin(v0) * radius, sin(a) * cos(v0) * radius };

			LibMath::Vector3 h1 = { cos(a) * cos(v1) * radius, sin(v1) * radius, sin(a) * cos(v1) * radius };

			AddLine(transformPoint(topLocal + h0), transformPoint(topLocal + h1));

			AddLine(transformPoint(bottomLocal - h0), transformPoint(bottomLocal - h1));
		}
	}
}

void DebugRenderer::AddLine(LibMath::Vector3 start, LibMath::Vector3 end)
{
	m_lines.push_back(Line(start, end));
}

void DebugRenderer::DrawLines(LibMath::Matrix4 viewProj)
{
	if (m_lines.empty())
		return;

	m_shader->Use();
	m_shader->SetUniform("uViewProjection", viewProj);
	m_shader->SetUniform("uColor", { 1,0,0 });

	// Upload to GPU
	m_rhi->UpdateBuffer(m_vbo, m_lines.data(), m_lines.size() * sizeof(Line));

	// Draw
	m_rhi->BindVAO(m_vao);
	m_rhi->DrawLineArrays((uint32_t)(m_lines.size() * 2));

	// Clear for next frame
	m_lines.clear();
}


DebugRenderer::DebugRenderer(Apex::Resources::ResourceHandle<Shader> shader, IRHI* rhi) : m_shader(shader), m_rhi(rhi)
{
	m_vao = m_rhi->CreateVAO();
	m_rhi->BindVAO(m_vao);
	m_vbo = m_rhi->CreateBuffer(BufferType::Vertex, m_lines.data(), sizeof(LibMath::Vector3) * 500'000, false);
	m_rhi->SetupVector3Layout();
}