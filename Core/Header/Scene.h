#ifndef SCENE
#define SCENE

#include "MeshRenderer.h"
#include "RHI.h"
#include "RenderPassInfo.h"

#include "ResourceManager.h"
#include "Object.h"

#include "Physic.h"

#include "Skybox.h"

#include "WaypointGraph.h"

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

		std::unique_ptr<Scene> Clone() const;

		void SetSkybox(std::unique_ptr<Skybox> skybox) { m_skybox = std::move(skybox); }

		void SetName(std::string const& name) { m_name = name; }
		std::string const& GetName() { return m_name; }

		Lighting::VisibleLights GetVisibleLights() const;

		Data::Object* CreateObject();
		Data::Object* CreateObjectRunTime();
		const std::vector<std::unique_ptr<Data::Object>>& GetObjects() const;
		void AddObject(std::unique_ptr<Data::Object> obj);
		void DestroyObject(size_t objId);

		void MarkDirty() const;
		void ClearDirty() const { m_isDirty = false; }
		bool IsDirty() const { return m_isDirty; }

		void Start();
		void Update(float deltaTime_s);
		void LateUpdate(float deltaTime_s);
		void FixedUpdate(float deltaTime_s);
		void ComputePass(RenderPassInfo passInfo);
		void Render(RenderPassInfo passInfo);
		
		template<typename T>
		std::vector<T*> GetComponents();

		Data::Object* GetObjectWithId(size_t id);
		Data::Object* GetObjectAt(size_t idx) { return idx < m_objects.size() ? m_objects[idx].get() : nullptr; }

		Camera* GetMainCamera() const { return m_mainCamera; }
		void	SetMainCamera(Camera* camera) { m_mainCamera = camera; }
		void	SetShader(Resources::ResourceHandle<Shader> shader) { m_shader = shader; }
		Resources::ResourceHandle<Shader>	GetShader() const { return m_shader; }

		void InsertBefore(Apex::Data::Object* dragged, Apex::Data::Object* target);
		void InsertAfter(Apex::Data::Object* dragged, Apex::Data::Object* target);
		Physic::PhysicSystem* GetPhysic() { return &m_physics; }
		Pathfinding::WaypointGraph* GetGraph() { return &m_graph; }

		void Save();
		void SetOnMarkedDirtyCallback(std::function<void()> callback);
	private:
		// clone helpers
		void CloneObjects(std::unique_ptr<Scene>& copy) const;
		void ReparentObjects(std::unique_ptr<Scene>& copy) const;

		void DestroyPendingObject();
		void DestroyObject(Apex::Data::Object* obj);

		Physic::PhysicSystem m_physics;
		Pathfinding::WaypointGraph m_graph;

		std::string m_name;

		std::vector<std::unique_ptr<Apex::Data::Object>> m_objects;
		std::vector<std::unique_ptr<Apex::Data::Object>> m_createdObjects;
		Resources::ResourceHandle<Shader> m_shader;
		std::unique_ptr<Skybox> m_skybox;

		Camera* m_mainCamera = nullptr;

		std::function<void()> m_onMarkedDirty;
		mutable bool m_isDirty = false;
	};

	template<typename T>
	std::vector<T*> Scene::GetComponents()
	{
		std::vector<T*> comps;

		for (auto& obj : m_objects)
		{
			auto objComp = obj->GetComponentsByType<T>();
			if (objComp.empty())
				continue;
			comps.insert(comps.end(), objComp.begin(), objComp.end());
		}

		return comps;
	}
}

#endif