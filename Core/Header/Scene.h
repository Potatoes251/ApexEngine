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

		Data::Object* CreateObject();
		const std::vector<std::unique_ptr<Data::Object>>& GetObjects() const;
		void AddObject(std::unique_ptr<Data::Object> obj);
		void DestroyObject(size_t objId);

		void MarkDirty();
		void ClearDirty() { m_isDirty = false; }
		bool IsDirty() const { return m_isDirty; }

		void Start();
		void Update(float deltaTime_s);
		void LateUpdate(float deltaTime_s);
		void FixedUpdate(float deltaTime_s);
		void Render(RenderPassInfo passInfo);
		
		Data::Object* GetObjectWithId(size_t id);
		Data::Object* GetObjectAt(size_t idx) { return idx < m_objects.size() ? m_objects[idx].get() : nullptr; }

		Camera* GetMainCamera() const { return m_mainCamera; }
		void	SetMainCamera(Camera* camera) { m_mainCamera = camera; }

		void InsertBefore(Apex::Data::Object* dragged, Apex::Data::Object* target);
		void InsertAfter(Apex::Data::Object* dragged, Apex::Data::Object* target);
		Physic::PhysicSystem* GetPhysic() { return &m_physics; }

		void Save(const std::string& path);
		void SetOnMarkedDirtyCallback(std::function<void()> callback);
	private:
		void DestroyPendingObject();
		void DestroyObject(Apex::Data::Object* obj);

		Physic::PhysicSystem m_physics;

		std::vector<std::unique_ptr<Apex::Data::Object>> m_objects;
		std::unique_ptr<Skybox> m_skybox;

		Camera* m_mainCamera = nullptr;

		std::function<void()> m_onMarkedDirty;
		bool m_isDirty = false;
	};
}

#endif