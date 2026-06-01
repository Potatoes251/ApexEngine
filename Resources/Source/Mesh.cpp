#include "Mesh.h"

#include "MeshLoader.h"

using namespace Apex::Rendering;

Mesh::Mesh(Mesh&& other) noexcept
    : m_vertices(std::move(other.m_vertices)),
    m_indices(std::move(other.m_indices)),
    m_VAO(other.m_VAO),
    m_VAOSkinned(other.m_VAOSkinned),
    m_VBO(other.m_VBO),
    m_EBO(other.m_EBO),
    m_indexCount(other.m_indexCount),
    m_vertexCount(other.m_vertexCount),
    m_name(std::move(other.m_name)),
    m_inputVertices(std::move(other.m_inputVertices)),
    m_outputVertices(std::move(other.m_outputVertices))
{
    // Null out the source's GPU handles so its destructor doesn't delete them
    other.m_VAO = 0;
    other.m_VAOSkinned = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_indexCount = 0;
    other.m_vertexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this == &other)
        return *this;

    // Release current GPU resources before overwriting
    if (m_VAO) m_rhi->DeleteVAO(m_VAO);
    if (m_VAOSkinned) m_rhi->DeleteVAO(m_VAOSkinned);
    if (m_EBO) m_rhi->DeleteBuffer(m_EBO);    
    if (m_VBO) m_rhi->DeleteBuffer(m_VBO);

    m_vertices = std::move(other.m_vertices);
    m_indices = std::move(other.m_indices);
    m_name = std::move(other.m_name);
    m_inputVertices = std::move(other.m_inputVertices);
    m_outputVertices = std::move(other.m_outputVertices);
    m_VAO = other.m_VAO;
    m_VAOSkinned = other.m_VAOSkinned;
    m_VBO = other.m_VBO;
    m_EBO = other.m_EBO;
    m_indexCount = other.m_indexCount;
    m_vertexCount = other.m_vertexCount;

    other.m_VAO = 0;
    other.m_VAOSkinned = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_indexCount = 0;
    other.m_vertexCount = 0;

    return *this;
}

Mesh::~Mesh()
{
    if (m_VAO) m_rhi->DeleteVAO(m_VAO);
    if (m_VAOSkinned) m_rhi->DeleteVAO(m_VAOSkinned);
    if (m_EBO) m_rhi->DeleteBuffer(m_EBO);
    if (m_VBO) m_rhi->DeleteBuffer(m_VBO);
}

void Mesh::UploadToGpu()
{
	m_vertexCount = static_cast<uint32_t>(m_vertices.size());
	m_indexCount = static_cast<uint32_t>(m_indices.size());

	m_inputVertices = Ssbo(m_rhi, m_vertices.data(), m_vertexCount * sizeof(Vertex), 0);
	m_outputVertices = Ssbo(m_rhi, nullptr, m_vertexCount * sizeof(Vertex), 1);

	m_VAO = m_rhi->CreateVAO();
	m_rhi->BindVAO(m_VAO);
	m_VBO = m_rhi->CreateBuffer(BufferType::Vertex, m_vertices.data(), m_vertexCount * sizeof(Vertex));
	m_EBO = m_rhi->CreateBuffer(BufferType::Index, m_indices.data(), m_indexCount * sizeof(uint32_t));
	m_rhi->SetupVertexLayout();

	m_VAOSkinned = m_rhi->CreateVAO();
	m_rhi->BindVAO(m_VAOSkinned);
	m_rhi->BindBuffer(m_outputVertices.GetId(), BufferType::Vertex);
	m_rhi->BindBuffer(m_EBO, BufferType::Index);
	m_rhi->SetupVertexLayout();
}

void Mesh::ClearVertices()
{
	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_indices.clear();
	m_indices.shrink_to_fit();
}

void Mesh::BindForCompute() const
{
	m_inputVertices.Bind();
	m_outputVertices.Bind();
}

void Mesh::Draw(bool skinned) const
{
	m_rhi->BindVAO(skinned ? m_VAOSkinned : m_VAO);
	m_rhi->DrawIndexed(m_indexCount);
}

void Mesh::DrawLine() const
{
	m_rhi->BindVAO(m_VAO);
	m_rhi->DrawLineArrays(m_vertexCount);
}
