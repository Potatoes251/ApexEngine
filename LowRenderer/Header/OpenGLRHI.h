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
		void SetClearColor(float r, float g, float b, float a = 1.f) override;
		void Clear() override;
		void ClearDepth() override;

		RHIVAOHandle	CreateVAO() override;
		void			BindVAO(RHIVAOHandle vao) override;
		void			DeleteVAO(RHIVAOHandle vao) override;

		RHIBufferHandle		CreateBuffer(BufferType type, const void* data, size_t sizeBytes, bool isStatic = true) override;
		void				UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) override;
		void				BindBuffer(RHIBufferHandle buffer, BufferType type) override;
		void				DeleteBuffer(RHIBufferHandle buffer) override;
		
		RHIFrameBufferHandle	CreateFrameBuffer(unsigned width, unsigned height, RHITextureHandle& texture_out) override;
		RHIFrameBufferHandle	CreateEmptyFrameBuffer(RHITextureHandle texture, unsigned width, unsigned height) override;
		void					BindFrameBuffer(RHIFrameBufferHandle buffer, unsigned viewportWidth, unsigned viewportHeight) override;
		void					UnBindFrameBuffer() override;
		void					DeleteFrameBuffer(RHIFrameBufferHandle buffer) override;
		int						GetPixelData(int x, int y) override;

		RHIUniformBufferHandle	CreateUniformBuffer(unsigned int bindingPoint, size_t sizeBytes) override;
		void					BindUniformBuffer(RHIUniformBufferHandle buffer, size_t sizeBytes, void* data) override;
		void					DeleteUniformBuffer(RHIUniformBufferHandle buffer) override;

		RHITextureHandle	CreateTexture(const TextureDescription& desc) override;
		RHITextureHandle	CreateCubemap(const std::vector<TextureDescription>& desc) override;
		RHITextureHandle	CreateShadowMap(unsigned width, unsigned height) override;
		RHITextureHandle	CreateShadowCubeMap(unsigned width, unsigned height) override;
		void				AttachDepthTexture(RHITextureHandle depthTexture) override;
		void				AttachDepthCubeTexture(RHITextureHandle depthTexture) override;
		void				BindTexture(RHITextureHandle texture, uint32_t unit = 0) override;
		void				DeleteTexture(RHITextureHandle texture) override;

		RHIShaderHandle		CreateShader(const ShaderSource& source) override;
		RHIShaderHandle		CreateShaderGeo(const ShaderSource& source) override;
		void				BindShader(RHIShaderHandle shader) override;
		void				DeleteShader(RHIShaderHandle shader) override;

		void SetUniformInt(RHIShaderHandle shader, const std::string& name, int value) override;
		void SetUniformFloat(RHIShaderHandle shader, const std::string& name, float value) override;
		void SetUniformVec3(RHIShaderHandle shader, const std::string& name, const LibMath::Vector3& vec) override;
		void SetUniformMat4(RHIShaderHandle shader, const std::string& name, const LibMath::Matrix4& matrix) override;

		void SetupVertexLayout() override;
		void SetupVector3Layout() override;
		void DrawEmpty() override;
		void DrawIndexed(uint32_t indexCount) override;
		void DrawArrays(uint32_t vertexCount) override;
		void DrawLineArrays(uint32_t vertexCount) override;
		void DrawLineLoop(uint32_t vertexCount) override;
		void SetDepthTest(bool enabled) override;
		void SetDepthTestWrite(bool enabled) override;

		uint32_t GetBuffer(RHIBufferHandle handle)  const;
		uint32_t GetFrameBuffer(RHIFrameBufferHandle handle)  const;
		uint32_t GetuniformeBuffer(RHIUniformBufferHandle handle)  const;
		uint32_t GetVAO(RHIVAOHandle handle)		const;
		uint32_t GetTexture(RHITextureHandle handle) const override;
		uint32_t GetShader(RHIShaderHandle handle)  const;

	private:
		std::unordered_map<RHIBufferHandle, GLuint>			m_buffers;
		std::unordered_map<RHIFrameBufferHandle, GLuint>	m_frameBuffers;
		std::unordered_map<RHIUniformBufferHandle, GLuint>	m_uniformBuffers;
		std::unordered_map<RHIVAOHandle, GLuint>			m_vaos;
		std::unordered_map<RHITextureHandle, GLuint>		m_textures;
		std::unordered_map<RHIShaderHandle, GLuint>			m_shaders;

		uint32_t m_nextHandle = 1;

		uint32_t NewHandle() { return m_nextHandle++; }


		static GLuint       CompileShader(const std::string& path, GLenum type);
		static std::string  ReadFile(const std::string& path);
	};
}

#endif