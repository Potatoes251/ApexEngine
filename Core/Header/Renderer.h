#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"

namespace Apex::Rendering
{
	class Renderer
	{
	public:
		explicit Renderer(IRHI* rhi);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void SetTexture(Apex::Resources::ResourceHandle texture);
		void SetShader(Apex::Resources::ResourceHandle shader);

		void Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection, 
					const LibMath::Vector3& cameraPosition, float deltaTime);

		float m_rotationSpeed = 5.f;

	private:
		IRHI* m_rhi = nullptr;
		RHIVAOHandle m_vao = RHI_INVALID;
		RHIBufferHandle m_vbo = RHI_INVALID;
		RHIBufferHandle m_ebo = RHI_INVALID;

		Apex::Resources::ResourceHandle m_textureHandle;
		Apex::Resources::ResourceHandle m_shaderHandle;
	};
}

#endif