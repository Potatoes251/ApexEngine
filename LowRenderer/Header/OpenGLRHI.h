#ifndef OPENGLRHI
#define OPENGLRHI

#include "RHI.h"

#include <glad/glad.h>
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

		RHIBufferHandle		CreateBuffer(BufferType type, const void* data, size_t sizeBytes, bool isStatic = true) override;
		void				UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) override;
		void				BindBuffer(RHIBufferHandle buffer, BufferType type) override;
		void				DeleteBuffer(RHIBufferHandle buffer) override;

		RHITextureHandle	CreateTexture(const TextureDescription& desc) override;
		void				BindTexture(RHITextureHandle texture, uint32_t unit = 0) override;
		void				DeleteTexture(RHITextureHandle texture) override;

		RHIShaderHandle		CreateShader(const ShaderSource& source) override;
		void				BindShader(RHIShaderHandle shader) override;
		void				DeleteShader(RHIShaderHandle shader) override;

		void SetUniformInt(RHIShaderHandle shader, const std::string& name, int value) override;
		void SetUniformFloat(RHIShaderHandle shader, const std::string& name, float value) override;
		void SetUniformVec3(RHIShaderHandle shader, const std::string& name, const LibMath::Vector3& vec) override;
		void SetUniformMat4(RHIShaderHandle shader, const std::string& name, const LibMath::Matrix4& matrix) override;

		void SetupVertexLayout() override;
		void SetupVector3Layout() override;
		void DrawIndexed(int32_t indexCount) override;
		void DrawArrays(int32_t vertexCount) override;
		void DrawLineArrays(int32_t vertexCount) override;
		void SetDepthTest(bool enabled) override;

	private:
		std::unordered_map<RHIBufferHandle, GLuint>	 m_buffers;
		std::unordered_map<RHIVAOHandle, GLuint>	 m_vaos;
		std::unordered_map<RHITextureHandle, GLuint> m_textures;
		std::unordered_map<RHIShaderHandle, GLuint>	 m_shaders;

		uint32_t m_nextHandle = 1;

		uint32_t NewHandle() { return m_nextHandle++; }

		GLuint GetGLBuffer(RHIBufferHandle handle)  const;
		GLuint GetGLVAO(RHIVAOHandle handle)		const;
		GLuint GetGLTexture(RHITextureHandle handle) const;
		GLuint GetGLShader(RHIShaderHandle handle)  const;

		static GLuint       CompileShader(const std::string& path, GLenum type);
		static std::string  ReadFile(const std::string& path);
	};
}

#endif