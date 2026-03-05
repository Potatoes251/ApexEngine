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
		glBindVertexArray(vao);
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

	RHIBufferHandle OpenGLRHI::CreateBuffer(BufferType type, const void* data, size_t sizeBytes) 
	{
		GLuint glBuffer;
		glGenBuffers(1, &glBuffer);
		glBindBuffer(GetGLBufferType(type), glBuffer);
		glBufferData(GetGLBufferType(type), sizeBytes, data, GL_STATIC_DRAW);
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

	// ── Uniforms ─────────────────────────────────────────────────
	void OpenGLRHI::SetUniformInt(GLuint program, const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(program, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLRHI::SetUniformFloat(GLuint program, const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(program, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLRHI::SetUniformVec3(GLuint program, const std::string& name, const LibMath::Vector3& vec)
	{
		GLint location = glGetUniformLocation(program, name.c_str());
		const LibMath::Vector3& vector = vec;
		glUniform3f(location, vector[0], vector[1], vector[2]);
	}

	void OpenGLRHI::SetUniformMat4(GLuint program, const std::string& name, const LibMath::Matrix4& matrix)
	{
		GLint location = glGetUniformLocation(program, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
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

	void OpenGLRHI::SetDepthTest(bool enabled) 
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	// ── Helpers ───────────────────────────────────────────────────
	GLuint OpenGLRHI::GetGLBuffer(RHIBufferHandle handle) const { return m_buffers.at(handle); }
	GLuint OpenGLRHI::GetGLVAO(RHIVAOHandle handle) const { return m_vaos.at(handle); }
}