#include "Shader.h"

#include <sstream>
#include <fstream>
#include <iostream>

void Shader::UploadToGpu()
{
	std::vector<std::string> files;

	std::stringstream ss(m_name);
	std::string item;

	while (std::getline(ss, item, '|')) // split by '|'
	{
		files.push_back(item);
	}
	if (!CreateShader(files))
	{
		SetState(ResourceState::Failed);
		return;
	}
}

void Shader::Use() const
{
	glUseProgram(m_shaderProgram);
}

bool Shader::CreateShader(std::vector<std::string> const& files)
{
	if (files.size() != 2 && files.size() != 3) return false;

	m_shaderProgram = glCreateProgram();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint geometryShader = 0;

	if (!InitShader(files[0], vertexShader) ||
		!InitShader(files[1], fragmentShader))
	{
		Cleanup(vertexShader, fragmentShader, 0);
		return false;
	}

	if (files.size() == 3)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		if (!InitShader(files[2], geometryShader))
		{
			Cleanup(vertexShader, fragmentShader, geometryShader);
			return false;
		}
	}

	if (!Link(vertexShader, fragmentShader, geometryShader))
	{
		Cleanup(vertexShader, fragmentShader, geometryShader);
		return false;
	}

	return true;
}

bool Shader::InitShader(std::string const& shaderFile, GLuint& shader) const
{
	std::ifstream file("../../Assets/Shaders/" + shaderFile);	// open the file
	if (!file.is_open())
	{
		std::cout << "Failed to open file : " << shaderFile << std::endl;
		return false;
	}

	std::stringstream shaderStream;
	shaderStream << file.rdbuf();

	std::string shaderString = shaderStream.str();

	char const* shaderSrc = shaderString.c_str();

	glShaderSource(shader, 1, &shaderSrc, NULL);

	glCompileShader(shader);

	GLint result;
	GLchar infolog[1024];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(shader, sizeof(infolog), NULL, infolog);
		std::cout << "Error! shader " << shaderFile << " failed to compile." << infolog << std::endl;
		return false;
	}

	return true;
}

bool Shader::Link(GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader) const
{
	glAttachShader(m_shaderProgram, vertexShader);
	if (geometryShader)
		glAttachShader(m_shaderProgram, geometryShader);
	glAttachShader(m_shaderProgram, fragmentShader);

	glLinkProgram(m_shaderProgram);

	GLint result = 0;
	GLchar infolog[1024];

	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(m_shaderProgram, sizeof(infolog), NULL, infolog);
		std::cout << "Error! shader failed to link." << infolog << std::endl;
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (geometryShader)
		glDeleteShader(geometryShader);

	return true;
}


void Shader::Cleanup(GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader)
{
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geometryShader);
	glDeleteProgram(m_shaderProgram);
	m_shaderProgram = 0;
}