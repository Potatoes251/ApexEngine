#include "ComputeShader.h"


void ComputeShader::UploadToGpu()
{
	m_shader = m_rhi->CreateComputeShader(m_path);
	SetState(ResourceState::Uploaded);
}

void ComputeShader::Use()
{
	m_rhi->BindShader(m_shader);
}

void ComputeShader::Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
{
	m_rhi->DispatchCompute(groupX, groupY, groupZ);
}

void ComputeShader::SetUniform(std::string_view const& name, int value)
{
	m_rhi->SetUniformInt(m_shader, name, value);
}

void ComputeShader::SetUniform(std::string_view const& name, float value)
{
	m_rhi->SetUniformFloat(m_shader, name, value);
}

void ComputeShader::SetUniform(std::string_view const& name, LibMath::Vector3 const& value)
{
	m_rhi->SetUniformVec3(m_shader, name, value);
}

void ComputeShader::SetUniform(std::string_view const& name, LibMath::Matrix4 const& value)
{
	m_rhi->SetUniformMat4(m_shader, name, value);
}

void ComputeShader::SetUniform(std::string_view const& name, LibMath::Matrix4 const& value, int count)
{
	m_rhi->SetUniformMat4Array(m_shader, name, value, count);
}