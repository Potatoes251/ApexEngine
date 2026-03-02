#ifndef SCENE
#define SCENE

#include "MeshRenderer.h"
#include "RHI.h"

#include "ResourceManager.h"
#include "Object.h"

namespace Apex::Rendering
{
	class Scene
	{
	public:
		Scene(IRHI* rhi, Apex::Core::ThreadPool& threadPool);
		~Scene() = default;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Apex::Data::Object* AddMesh(const std::string& meshFile,
									const std::string& textureFile,
									const std::string& shaderName);

		MeshRenderer& GetRenderer(size_t index) { return m_renderers[index]; }
		const MeshRenderer& GetRenderer(size_t index) const { return m_renderers[index]; }


		void Update(const LibMath::Matrix4& view, const LibMath::Matrix4& projection,
					const LibMath::Vector3& cameraPosition, float deltaTime);
		
	private:
		IRHI* m_rhi = nullptr;
		Apex::Resources::ResourceManager   m_resourceManager;
		std::vector<MeshRenderer>          m_renderers;
	};
}

#endif