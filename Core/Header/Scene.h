#ifndef SCENE
#define SCENE

#include "MeshRenderer.h"
#include "RHI.h"
#include "RenderPassInfo.h"

#include "ResourceManager.h"
#include "Object.h"

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

		void Update(float deltaTime);
		void Render(RenderPassInfo passInfo);
		
		Object* GetObjectAt(size_t idx) { return idx < m_objects.size() ? m_objects[idx].get() : nullptr; }

	private:
		std::vector<std::unique_ptr<Object>> m_objects;
	};
}

#endif