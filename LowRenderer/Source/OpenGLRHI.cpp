#include "OpenGLRHI.h"

#include <fstream>
#include <sstream>

namespace Apex::Rendering
{
	IRHI* CreateOpenGLRHI() { return new OpenGLRHI(); }

	// ── Constructor ─────────────────────────────────────────────
	OpenGLRHI::OpenGLRHI()
	{
		Initialize();
	}

	// ── Init / Clear ─────────────────────────────────────────────
	void OpenGLRHI::Initialize()
	{
		glEnable(GL_DEPTH_TEST);
	}
	void OpenGLRHI::Clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// ── VAO ───────────────────────────────────────────────────────
	RHIVAOHandle OpenGLRHI::CreateVAO() 
	{
		GLuint glVAO;
		glGenVertexArrays(1, &glVAO);
		RHIVAOHandle vao = NewHandle();
		m_vaos[vao] = glVAO;
		return vao;
	}
	void OpenGLRHI::BindVAO(RHIVAOHandle vao) 
	{
		glBindVertexArray(GetGLVAO(vao));
	}
	void OpenGLRHI::DeleteVAO(RHIVAOHandle vao) 
	{
		RHIVAOHandle vaoToDelete = GetGLVAO(vao);
		glDeleteVertexArrays(1, &vaoToDelete);
		m_vaos.erase(vao);
	}

	// ── Buffers ─────────────────────────────────────────────────
	static GLenum GetGLBufferType(BufferType type)
	{
		switch (type) 
		{
		case BufferType::Vertex: return GL_ARRAY_BUFFER;
		case BufferType::Index:  return GL_ELEMENT_ARRAY_BUFFER;
		default:                 return GL_ARRAY_BUFFER; // Default to vertex buffer
		}
	}

	RHIBufferHandle OpenGLRHI::CreateBuffer(BufferType type, const void* data, size_t sizeBytes, bool isStatic) 
	{
		GLuint glBuffer;
		glGenBuffers(1, &glBuffer);
		glBindBuffer(GetGLBufferType(type), glBuffer);
		glBufferData(GetGLBufferType(type), sizeBytes, data, isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		RHIBufferHandle buffer = NewHandle();
		m_buffers[buffer] = glBuffer;
		return buffer;
	}

	void OpenGLRHI::UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) 
	{
		GLuint glBuffer = GetGLBuffer(buffer);
		glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeBytes, data);
	}

	void OpenGLRHI::BindBuffer(RHIBufferHandle buffer, BufferType type) 
	{
		glBindBuffer(GetGLBufferType(type), GetGLBuffer(buffer));
	}

	void OpenGLRHI::DeleteBuffer(RHIBufferHandle buffer) 
	{
		GLuint glBuffer = GetGLBuffer(buffer);
		glDeleteBuffers(1, &glBuffer);
		m_buffers.erase(buffer);
	}

	// ── Textures ─────────────────────────────────────────────────
	RHITextureHandle OpenGLRHI::CreateTexture(const TextureDescription& desc)
	{
		GLuint glTexture;
		glGenTextures(1, &glTexture);
		glBindTexture(GL_TEXTURE_2D, glTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		const GLenum internalFmt = (desc.m_channels == 4) ? GL_RGBA8 : GL_RGB8;
		const GLenum pixelFmt = (desc.m_channels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, desc.m_width, desc.m_height, 0, pixelFmt, GL_UNSIGNED_BYTE, desc.m_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		RHITextureHandle texture = NewHandle();
		m_textures[texture] = glTexture;
		return texture;
	}

	void OpenGLRHI::BindTexture(RHITextureHandle texture, GLuint unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, GetGLTexture(texture));
	}

	void OpenGLRHI::DeleteTexture(RHITextureHandle texture)
	{
		GLuint glTexture = GetGLTexture(texture);
		glDeleteTextures(1, &glTexture);
		m_textures.erase(texture);
	}

	// ── Shaders ──────────────────────────────────────────────────
	RHIShaderHandle OpenGLRHI::CreateShader(const ShaderSource& source)
	{
		GLuint vertexShader = CompileShader(source.vertexPath, GL_VERTEX_SHADER);
		GLuint fragmentShader = CompileShader(source.fragmentPath, GL_FRAGMENT_SHADER);
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		RHIShaderHandle shader = NewHandle();
		m_shaders[shader] = program;
		return shader;
	}

	void OpenGLRHI::BindShader(RHIShaderHandle shader)
	{
		glUseProgram(GetGLShader(shader));
	}

	void OpenGLRHI::DeleteShader(RHIShaderHandle shader)
	{
		glDeleteProgram(GetGLShader(shader));
		m_shaders.erase(shader);
	}

	// ── Uniforms ─────────────────────────────────────────────────
	void OpenGLRHI::SetUniformInt(RHIShaderHandle shader, const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(GetGLShader(shader), name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLRHI::SetUniformFloat(RHIShaderHandle shader, const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(GetGLShader(shader), name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLRHI::SetUniformVec3(RHIShaderHandle shader, const std::string& name, const LibMath::Vector3& vec)
	{
		GLint location = glGetUniformLocation(GetGLShader(shader), name.c_str());
		const LibMath::Vector3& vector = vec;
		glUniform3f(location, vector[0], vector[1], vector[2]);
	}

	void OpenGLRHI::SetUniformMat4(RHIShaderHandle shader, const std::string& name, const LibMath::Matrix4& matrix)
	{
		GLint location = glGetUniformLocation(GetGLShader(shader), name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
	}

	void OpenGLRHI::SetupVertexLayout()
	{
		glEnableVertexAttribArray(0); // Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_position));
		glEnableVertexAttribArray(1); // Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
		glEnableVertexAttribArray(2); // UV
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_uv));
	}

	void OpenGLRHI::SetupVector3Layout()
	{
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LibMath::Vector3), (void*)0);
	}

	// ── Draw ───────────────────────────────────────────────────────
	void OpenGLRHI::DrawIndexed(GLsizei indexCount) 
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRHI::DrawArrays(GLsizei vertexCount) 
	{
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void OpenGLRHI::DrawLineArrays(GLsizei vertexCount)
	{
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRHI::SetDepthTest(bool enabled) 
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	// ── Helpers ───────────────────────────────────────────────────
	GLuint OpenGLRHI::GetGLBuffer(RHIBufferHandle handle) const { return m_buffers.at(handle); }
	GLuint OpenGLRHI::GetGLTexture(RHITextureHandle handle) const { return m_textures.at(handle); }
	GLuint OpenGLRHI::GetGLShader(RHIShaderHandle handle) const { return m_shaders.at(handle); }
	GLuint OpenGLRHI::GetGLVAO(RHIVAOHandle handle) const { return m_vaos.at(handle); }

	GLuint OpenGLRHI::CompileShader(const std::string& path, GLenum type)
	{
		std::string source = ReadFile(path);
		const char* sourceCStr = source.c_str();
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &sourceCStr, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		return shader;
	}

	std::string OpenGLRHI::ReadFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			std::cerr << "ERROR::FILE::COULD_NOT_OPEN " << path << std::endl;
			return "";
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
}