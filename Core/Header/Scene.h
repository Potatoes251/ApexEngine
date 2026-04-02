#ifndef SCENE
#define SCENE

#include "MeshRenderer.h"
#include "RHI.h"
#include "RenderPassInfo.h"

#include "ResourceManager.h"
#include "Object.h"

#include "Physic.h"

#include "Skybox.h"

#include "Lighting/VisibleLights.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Rendering
{
	class Camera;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		std::unique_ptr<Scene> Clone();

		void SetSkybox(std::unique_ptr<Skybox>& skybox) { m_skybox = std::move(skybox); }

		Lighting::VisibleLights GetVisibleLights();

		Object* CreateObject();
		const std::vector<std::unique_ptr<Object>>& GetObjects() const;
		void DestroyObject(Object* obj);

		void Start();
		void Update(float deltaTime_s);
		void FixedUpdate(float deltaTime_s);
		void Render(RenderPassInfo passInfo);
		
		Object* GetObjectAt(size_t idx) { return idx < m_objects.size() ? m_objects[idx].get() : nullptr; }

		Camera* GetMainCamera() const { return m_mainCamera; }
		void	SetMainCamera(Camera* camera) { m_mainCamera = camera; }

		void InsertBefore(Object* dragged, Object* target);
		void InsertAfter(Object* dragged, Object* target);
		Physic::PhysicSystem* GetPhysic() { return &m_physics; }

		void Save(const std::string& path);
	private:
		Physic::PhysicSystem m_physics;

		std::vector<std::unique_ptr<Object>> m_objects;
		std::unique_ptr<Skybox> m_skybox;

		Camera* m_mainCamera = nullptr;
	};
}

#endif