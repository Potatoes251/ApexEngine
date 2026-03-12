#ifndef SCENE
#define SCENE

#include "MeshRenderer.h"
#include "RHI.h"

#include "ResourceManager.h"
#include "Object.h"

#include "Physic.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Rendering
{
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Object* CreateObject();
		const std::vector<std::unique_ptr<Object>>& GetObjects() const;

		void Update(float deltaTime);
		void Render(const LibMath::Matrix4& view,
			const LibMath::Matrix4& projection,
			const LibMath::Vector3& cameraPosition);

		void Save(const std::string& path, Apex::Physic::PhysicSystem& phys);
		
	private:
		std::vector<std::unique_ptr<Object>> m_objects;
	};
}

#endif