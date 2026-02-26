#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"

namespace Apex::Rendering
{
	class MeshRenderer
	{
	public:
		explicit MeshRenderer(IRHI* rhi);
		~MeshRenderer() = default;
		MeshRenderer(const MeshRenderer&) = delete;
		MeshRenderer& operator=(const MeshRenderer&) = delete;

		void SetMesh(Apex::Resources::ResourceHandle mesh);
		void SetTexture(Apex::Resources::ResourceHandle texture);
		void SetShader(Apex::Resources::ResourceHandle shader);

		void Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection, 
					const LibMath::Vector3& cameraPosition, float deltaTime);

		float m_rotationSpeed = 5.f;

	private:
		IRHI* m_rhi = nullptr;

		Apex::Resources::ResourceHandle m_meshHandle;
		Apex::Resources::ResourceHandle m_textureHandle;
		Apex::Resources::ResourceHandle m_shaderHandle;
	};
}

#endif