#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Rendering
{
	class Renderer
	{
	public:
		Renderer(IRHI* rhi);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void LoadTexture(const std::string& path);
		void Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection, 
					const LibMath::Vector3& cameraPosition, float deltaTime);

		float m_rotationSpeed = 5.f;

	private:
		IRHI* m_rhi = nullptr;
		RHIVAOHandle m_vao = RHI_INVALID;
		RHIBufferHandle m_vbo = RHI_INVALID;
		RHIBufferHandle m_ebo = RHI_INVALID;
		RHIShaderHandle m_shader = RHI_INVALID;
		RHITextureHandle m_texture = RHI_INVALID;
	};
}

#endif