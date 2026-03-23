#ifndef OPENGLRHI
#define OPENGLRHI

#include "RHI.h"

#include <unordered_map>
#include <string>

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

		RHITextureHandle	CreateTexture(const TextureDescription& desc) override;
		void				BindTexture(RHITextureHandle texture, GLuint unit = 0) override;
		void				DeleteTexture(RHITextureHandle texture) override;

		RHIShaderHandle		CreateShader(const ShaderSource& source) override;
		void				BindShader(RHIShaderHandle shader) override;
		void				DeleteShader(RHIShaderHandle shader) override;

		void SetUniformInt(GLuint shader, const std::string& name, int value) override;
		void SetUniformFloat(GLuint shader, const std::string& name, float value) override;
		void SetUniformVec3(GLuint shader, const std::string& name, const LibMath::Vector3& vec) override;
		void SetUniformMat4(GLuint shader, const std::string& name, const LibMath::Matrix4& matrix) override;
		void SetUniformMat4Array(GLuint shader, const std::string& name, const LibMath::Matrix4& matrices, int count) override;

		void SetupVertexLayout() override;
		void DrawIndexed(GLsizei indexCount) override;
		void DrawArrays(GLsizei vertexCount) override;
		void SetDepthTest(bool enabled) override;

	private:
		std::unordered_map<RHIBufferHandle, GLuint>		m_buffers;
		std::unordered_map<RHIVAOHandle, GLuint>		m_vaos;
		std::unordered_map<RHITextureHandle, GLuint>	m_textures;
		std::unordered_map<RHIShaderHandle, GLuint>		m_shaders;

		GLuint m_nextHandle = 1;

		GLuint NewHandle() { return m_nextHandle++; }

		GLuint GetGLBuffer(RHIBufferHandle handle)  const;
		GLuint GetGLVAO(RHIVAOHandle handle)		const;
		GLuint GetGLTexture(RHITextureHandle handle) const;
		GLuint GetGLShader(RHIShaderHandle handle)  const;

		static GLuint       CompileShader(const std::string& path, GLenum type);
		static std::string  ReadFile(const std::string& path);
	};
}

#endif