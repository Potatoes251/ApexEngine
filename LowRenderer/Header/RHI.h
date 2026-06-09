#ifndef RHI
#define RHI

// ============================================================
// RHI.h — Render Hardware Interface
// ============================================================

#include "Vertex.h"

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"

#include <string>
#include <vector>

namespace Apex::Rendering
{
	using RHIUniformBufferHandle = uint32_t;
	using RHIFrameBufferHandle = uint32_t;
	using RHIBufferHandle = uint32_t;
	using RHIVAOHandle = uint32_t;
	using RHIShaderHandle = uint32_t;
	using RHITextureHandle = uint32_t;

	constexpr uint32_t RHI_INVALID = 0;

	enum class BufferType
	{
		Vertex,
		Index
	};

	struct TextureDescription
	{
		int m_width = 0;
		int m_height = 0;
		int m_channels = 4; // Default to RGBA
		unsigned char* m_data = nullptr;
	};

	struct ShaderSource 
	{
		std::string m_vertexPath;
		std::string m_fragmentPath;
		std::string m_geometryPath;
	};
	// ── RHI abstract interface ────────────────────────────────────
	class IRHI
	{
	public:
		virtual ~IRHI() = default;

		virtual void Initialize() = 0;
		virtual void SetClearColor(float r, float g, float b, float a = 1.f) = 0;
		virtual void Clear() = 0;
		virtual void ClearDepth() = 0;
		virtual void SetCullingFace(bool front) = 0;
		virtual void SetCullingEnabled(bool enabled) = 0;

		// VAO
		virtual RHIVAOHandle	CreateVAO() = 0;
		virtual void			BindVAO(RHIVAOHandle vao) = 0;
		virtual void			DeleteVAO(RHIVAOHandle vao) = 0;

		// Buffers
		virtual RHIBufferHandle		CreateBuffer(BufferType type, const void* data, size_t sizeBytes, bool isStatic = true) = 0;
		virtual RHIBufferHandle		CreateSSBO(Vertex* data, size_t sizeBytes) = 0;
		virtual void				UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) = 0;
		virtual void				BindBuffer(RHIBufferHandle buffer, BufferType type) = 0;
		virtual void				BindStorageBuffer(uint32_t binding, RHIBufferHandle buffer) = 0;
		virtual void				DeleteBuffer(RHIBufferHandle buffer) = 0;
		
		virtual RHIFrameBufferHandle	CreateFrameBuffer(unsigned width, unsigned height, RHITextureHandle& texture_out) = 0;
		virtual RHIFrameBufferHandle	CreateEmptyFrameBuffer(RHITextureHandle texture, unsigned width, unsigned height) = 0;
		virtual void					BindFrameBuffer(RHIFrameBufferHandle buffer, unsigned viewportWidth, unsigned viewportHeight) = 0;
		virtual void					UnBindFrameBuffer(unsigned width = 1280, unsigned height = 720) = 0;
		virtual void					DeleteFrameBuffer(RHIFrameBufferHandle buffer) = 0;
		virtual int						GetPixelData(int x, int y) = 0;

		virtual RHIUniformBufferHandle	CreateUniformBuffer(unsigned int bindingPoint, size_t sizeBytes) = 0;
		virtual void					BindUniformBuffer(RHIUniformBufferHandle buffer, size_t sizeBytes, void* data) = 0;
		virtual void					DeleteUniformBuffer(RHIUniformBufferHandle buffer) = 0;

		// Textures
		virtual RHITextureHandle	CreateTexture(const TextureDescription& desc) = 0;
		virtual RHITextureHandle	CreateCubemap(const std::vector<TextureDescription>& desc) = 0;
		virtual RHITextureHandle	CreateShadowMap(unsigned width, unsigned height) = 0;
		virtual RHITextureHandle	CreateShadowCubeMap(unsigned width, unsigned height) = 0;
		virtual void				AttachDepthTexture(RHITextureHandle depthTexture) = 0;
		virtual void				AttachDepthCubeTexture(RHITextureHandle depthTexture) = 0;
		virtual void				BindTexture(RHITextureHandle texture, uint32_t unit = 0) = 0;
		virtual void				BindTextureCube(RHITextureHandle texture, uint32_t unit = 0) = 0;
		virtual void				DeleteTexture(RHITextureHandle texture) = 0;

		// Shaders
		virtual RHIShaderHandle		CreateShader(const ShaderSource& source) = 0;
		virtual RHIShaderHandle		CreateShaderGeo(const ShaderSource& source) = 0;
		virtual RHIShaderHandle		CreateComputeShader(const std::string& source) = 0;
		virtual void				BindShader(RHIShaderHandle shader) = 0;
		virtual void				DeleteShader(RHIShaderHandle shader) = 0;
		// waits for the gpu to finish its job in the compute shaders
		virtual void				Barrier() = 0;

		virtual void DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) = 0;

		// Uniforms (bound shader must be active)
		virtual void SetUniformInt(RHIShaderHandle program, const std::string_view& name, int value) = 0;
		virtual void SetUniformFloat(RHIShaderHandle program, const std::string_view& name, float value) = 0;
		virtual void SetUniformFloatArray(RHIShaderHandle shader, const std::string_view& name, const float& value, int count) = 0;
		virtual void SetUniformVec3(RHIShaderHandle program, const std::string_view& name, const LibMath::Vector3& vec) = 0;
		virtual void SetUniformVec4(RHIShaderHandle program, const std::string_view& name, const LibMath::Vector4& vec) = 0;
		virtual void SetUniformMat4(RHIShaderHandle program, const std::string_view& name, const LibMath::Matrix4& matrix) = 0;
		virtual void SetUniformMat4Array(RHIShaderHandle program, const std::string_view& name, const LibMath::Matrix4& matrix, int count) = 0;

		// Draw
		virtual void SetupVertexLayout() = 0;
		virtual void SetupVector3Layout() = 0;
		virtual void DrawEmpty() = 0;
		virtual void DrawIndexed(uint32_t indexCount) = 0;
		virtual void DrawArrays(uint32_t vertexCount) = 0;
		virtual void DrawLineArrays(uint32_t vertexCount) = 0;
		virtual void SetDepthTest(bool enabled) = 0;
		virtual void SetDepthTestWrite(bool enabled) = 0;

		virtual uint32_t GetBuffer(RHIBufferHandle handle)  const = 0;
		virtual uint32_t GetFrameBuffer(RHIFrameBufferHandle handle)  const = 0;
		virtual uint32_t GetVAO(RHIVAOHandle handle) const = 0;
		virtual uint32_t GetTexture(RHITextureHandle handle) const = 0;
		virtual uint32_t GetShader(RHIShaderHandle handle)  const = 0;
	};

	// Factory
	IRHI* CreateOpenGLRHI();

} // namespace Apex::Rendering

#endif