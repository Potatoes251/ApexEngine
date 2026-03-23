#include "Shader.h"

#include "RHI.h"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace Apex::Rendering;

void Shader::UploadToGpu()
{
	std::vector<std::string> files;

	std::stringstream ss(m_name);
	std::string item;

	while (std::getline(ss, item, '|')) // split by '|'
	{
		files.push_back(item);
	}
	if (files.size() != 2)
	{
		m_shader = RHI_INVALID;
		SetState(ResourceState::Failed);
		return;
	}

	m_shader = m_rhi->CreateShader(ShaderSource{ files[0],files[1] });

	if (m_shader == RHI_INVALID)
	{
		SetState(ResourceState::Failed);
	}
}

void Shader::Use() const
{
	if (m_rhi != nullptr)
	{
		m_rhi->BindShader(m_shader);
	}
	else
	{
		printf("rhi null\n");
	}
	
}

void Shader::SetUniform(std::string const& name, int value)
{
	m_rhi->SetUniformInt(m_shader, name, value);
}

void Shader::SetUniform(std::string const& name, float value)
{
	m_rhi->SetUniformFloat(m_shader, name, value);
}

void Shader::SetUniform(std::string const& name, LibMath::Vector3 const& value)
{
	m_rhi->SetUniformVec3(m_shader, name, value);
}

void Shader::SetUniform(std::string const& name, LibMath::Matrix4 const& value)
{
	m_rhi->SetUniformMat4(m_shader, name, value);
}

void Shader::SetUniformMat4Array(std::string const& name, std::vector<LibMath::Matrix4> const& value)
{
	m_rhi->SetUniformMat4Array(m_shader, name, *value.data(), static_cast<int>(value.size()));
}

