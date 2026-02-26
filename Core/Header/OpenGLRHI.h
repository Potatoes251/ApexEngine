#ifndef OPENGLRHI
#define OPENGLRHI

#include "RHI.h"

#include <unordered_map>

namespace Apex::Rendering
{
	class OpenGLRHI final : public IRHI
	{
	public:
		OpenGLRHI();
		~OpenGLRHI() = default;
		OpenGLRHI(const OpenGLRHI&) = delete;
		OpenGLRHI& operator=(const OpenGLRHI&) = delete;

		void Initialize() override;
		void Clear(float r, float g, float b, float a = 1.f) override;

		RHIVAOHandle	CreateVAO() override;
		void			BindVAO(RHIVAOHandle vao) override;
		void			DeleteVAO(RHIVAOHandle vao) override;

		RHIBufferHandle		CreateBuffer(BufferType type, const void* data, size_t sizeBytes) override;
		void				UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) override;
		void				BindBuffer(RHIBufferHandle buffer, BufferType type) override;
		void				DeleteBuffer(RHIBufferHandle buffer) override;

		void SetUniformInt(GLuint program, const std::string& name, int value) override;
		void SetUniformFloat(GLuint program, const std::string& name, float value) override;
		void SetUniformVec3(GLuint program, const std::string& name, const LibMath::Vector3& vec) override;
		void SetUniformMat4(GLuint program, const std::string& name, const LibMath::Matrix4& matrix) override;

		void SetupVertexLayout() override;
		void DrawIndexed(GLsizei indexCount) override;
		void DrawArrays(GLsizei vertexCount) override;
		void SetDepthTest(bool enabled) override;

	private:
		std::unordered_map<RHIBufferHandle, GLuint>		m_buffers;
		std::unordered_map<RHIVAOHandle, GLuint>		m_vaos;

		GLuint m_nextHandle = 1;

		GLuint NewHandle() { return m_nextHandle++; }

		GLuint GetGLBuffer(RHIBufferHandle handle)  const;
		GLuint GetGLVAO(RHIVAOHandle handle)     const;
	};
}

#endif