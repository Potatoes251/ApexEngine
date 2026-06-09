#include "Scene.h"
#include "SceneGraph.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "Camera.h"

#include "Rigidbody.h"
#include "BoxCollider.h"
#include "WaterVolume.h"
#include "DebugRenderer.h"

#include "Lighting/Lights.h"
#include "CharacterController.h"

#include "Waypoint.h"

#include "Log.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace Apex::Data;
using namespace Apex::Physic;

namespace Apex::Rendering
{
	std::unique_ptr<Scene> Scene::Clone() const
	{
		std::unique_ptr<Scene> copy = std::make_unique<Scene>();
		copy->m_skybox = std::make_unique<Skybox>(*m_skybox);
		copy->m_graph = m_graph;
		copy->m_shader = m_shader;
		copy->m_name = m_name;

		CloneObjects(copy);
		ReparentObjects(copy);

		for (auto const& obj : copy->m_objects)
		{
			Camera* cam = obj->GetComponent<Camera>();
			if (cam && cam->IsMainCamera())
			{
				copy->m_mainCamera = cam;
				break;
			}
		}

		return copy;
	}

	Lighting::VisibleLights Scene::GetVisibleLights() const
	{
		Lighting::VisibleLights visibleLights;

		for (std::unique_ptr<Object> const& obj : m_objects)
		{
			Lighting::DirectionalLightComponent* dirLight = obj->GetComponent<Lighting::DirectionalLightComponent>();

			if (dirLight)
				visibleLights.m_directionalLights.push_back(dirLight);

			Lighting::SpotLightComponent* spotLight = obj->GetComponent<Lighting::SpotLightComponent>();

			if (spotLight)
				visibleLights.m_spotLights.push_back(spotLight);

			Lighting::PointLightComponent* pointLight = obj->GetComponent<Lighting::PointLightComponent>();
			
			if (pointLight)
				visibleLights.m_pointLights.push_back(pointLight);
		}

		return visibleLights;
	}

	Object* Scene::CreateObject()
	{
		auto obj = std::make_unique<Object>();
		Object* ptr = obj.get();

		obj->SetOnChangedCallback([this]()
			{
				MarkDirty();
			});

		m_objects.push_back(std::move(obj));
		MarkDirty();

		return ptr;
	}

	Object* Scene::CreateObjectRunTime()
	{
		auto obj = std::make_unique<Object>();
		Object* ptr = obj.get();

		obj->SetOnChangedCallback([this]()
			{
				MarkDirty();
			});

		m_createdObjects.push_back(std::move(obj));
		MarkDirty();

		return ptr;
	}

	const std::vector<std::unique_ptr<Object>>& Scene::GetObjects() const
	{
		return m_objects;
	}

	void Scene::AddObject(std::unique_ptr<Object> obj)
	{
		m_objects.push_back(std::move(obj));
		MarkDirty();
	}

	void Scene::DestroyPendingObject()
	{
		std::vector<Object*> destroyQueue;

		for (std::unique_ptr<Object>& obj : m_objects)
		{
			if (obj->IsPendingDestroy())
			{
				destroyQueue.push_back(obj.get());
			}
		}

		for (Object* obj : destroyQueue)
		{
			DestroyObject(obj);
		}
	}

	void Scene::DestroyObject(Object* obj)
	{
		if (!obj)
			return;

		std::shared_ptr<Apex::SceneGraph::SceneNode> node = obj->GetSceneNode();

		if (node)
		{
			for (auto const& child : node->GetChildren())
			{
				if (child && child->GetObject())
					DestroyObject(child->GetObject());
			}
		}

		if (node && node->GetParent())
			node->Destroy();

		auto it = std::find_if(
			m_objects.begin(),
			m_objects.end(),
			[&](const std::unique_ptr<Object>& ptr)
			{
				return ptr.get() == obj;
			});

		if (it != m_objects.end())
		{
			RigidBodyComponent* body = obj->GetComponent<RigidBodyComponent>();
			if (body)
				m_physics.RemoveActor(*body);

			m_objects.erase(it);
			MarkDirty();
		}
	}

	void Scene::DestroyObject(size_t objId)
	{
		for (std::unique_ptr<Object>& obj : m_objects)
		{
			if (obj && obj->GetId() == objId)
			{
				DestroyObject(obj.get());
				return;
			}
		}
	}

	void Scene::MarkDirty() const
	{
		m_isDirty = true;
		if (m_onMarkedDirty)
			m_onMarkedDirty();
	}
		
	void Scene::Start()
	{
		for (auto& obj : m_objects)
		{
			obj->Start();

			Controller::CharacterController* character = obj->GetComponent<Controller::CharacterController>();

			if (character) 
			{
				character->SetPhysic(&m_physics);
			}

			if (m_mainCamera) continue;

			Camera* camera = obj->GetComponent<Camera>();
			if (camera && camera->IsMainCamera())
			{
				m_mainCamera = camera;
			}
		}
	}

	void Scene::Update(float deltaTime_s)
	{
		for (auto& obj : m_objects)
		{
			obj->Update(deltaTime_s);
		}
	}

	void Scene::LateUpdate(float deltaTime_s)
	{
		for (auto& obj : m_objects)
		{
			obj->LateUpdate(deltaTime_s);
		}

		for (auto& obj : m_createdObjects)
		{
			m_objects.push_back(std::move(obj));
		}
		m_createdObjects.clear();

		DestroyPendingObject();
	}

	void Scene::FixedUpdate(float deltaTime_s)
	{
		m_physics.Update(deltaTime_s);
		for (auto& obj : m_objects)
			obj->FixedUpdate(deltaTime_s);
	}

	void Scene::ComputePass(RenderPassInfo passInfo)
	{
		std::vector<MeshRenderer*> renderers = GetComponents<MeshRenderer>();

		if (passInfo.m_flags & PerformCompute)
		{
			for (MeshRenderer* renderer : renderers)
			{
				renderer->ComputePass();
			}
		}
		else
		{
			for (MeshRenderer* renderer : renderers)
			{
				renderer->SetSkinned(false);
			}
		}
	}

	void Scene::Render(RenderPassInfo passInfo)
	{
		if (!passInfo.m_shader.IsReady())
		{
			passInfo.m_shader = m_shader;
		}
		if (!passInfo.m_shader.IsReady()) return;

		passInfo.m_shader->Use();

		if (passInfo.m_rhi) passInfo.m_rhi->SetCullingFace(false);

		for (MeshRenderer* renderer : GetComponents<MeshRenderer>())
		{
			renderer->Render(passInfo);
		}
		if (passInfo.m_flags & RenderSkybox && passInfo.m_camera)
		{
			LibMath::Matrix4 view = passInfo.m_camera->GetViewMatrix();
			view[3][0] = 0;
			view[3][1] = 0;
			view[3][2] = 0;
			if (passInfo.m_rhi) passInfo.m_rhi->SetCullingFace(true);
			m_skybox->Render(passInfo.m_camera->GetProjection() * view);
		}
		if (passInfo.m_flags & ShowCollider)
		{
			DebugRenderer& dr = DebugRenderer::Get();
			for (WaterVolume* vol : GetComponents<WaterVolume>())
			{
				dr.AddBox(vol->GetOwner()->GetGlobalPosition(), {}, vol->GetHalfExtents());
			}
			m_physics.DrawColliders();
			m_graph.DebugRender();
		}
	}

	Object* Scene::GetObjectWithId(size_t id)
	{
		for (std::unique_ptr<Object>& obj : m_objects)
		{
			if (obj && obj->GetId() == id)
			{
				return obj.get();
			}
		}
		return nullptr;
	}

	void Scene::InsertBefore(Object* dragged, Object* target)
	{
		if (!dragged || !target || dragged == target)
			return;

		auto itDragged = std::find_if(m_objects.begin(), m_objects.end(),
			[&](auto& o) { return o.get() == dragged; });

		auto itTarget = std::find_if(m_objects.begin(), m_objects.end(),
			[&](auto& o) { return o.get() == target; });

		if (std::next(itDragged) == itTarget)
			return;

		if (itDragged == m_objects.end() || itTarget == m_objects.end())
			return;

		if (itDragged < itTarget)
			std::rotate(itDragged, itDragged + 1, itTarget);
		else
			std::rotate(itTarget, itDragged, itDragged + 1);

		MarkDirty();
	}

	void Scene::InsertAfter(Object* dragged, Object* target)
	{
		if (!dragged || !target || dragged == target)
			return;

		auto itDragged = std::find_if(m_objects.begin(), m_objects.end(),
			[&](auto& o) { return o.get() == dragged; });

		auto itTarget = std::find_if(m_objects.begin(), m_objects.end(),
			[&](auto& o) { return o.get() == target; });

		if (std::next(itTarget) == itDragged)
			return;

		if (itDragged == m_objects.end() || itTarget == m_objects.end())
			return;

		if (itDragged < itTarget)
			std::rotate(itDragged, itDragged + 1, itTarget + 1);
		else
			std::rotate(itTarget + 1, itDragged, itDragged + 1);

		MarkDirty();
	}

	void Scene::Save()
	{
		std::ofstream file(m_name);

		if (!file)
		{
			LOG_ERROR("Scene::Save Failed to open file : {}", m_name);
			return;
		}

		file << std::fixed;
		file << std::setprecision(6);

		file << "{\n";

		file << "\"objects\": [\n";

		for (const auto& obj : m_objects)
		{
			const auto& transform = obj->GetLocalTransform();

			auto const& pos = transform.getPosition();
			auto const& rot = transform.getRotation();
			auto const& scale = transform.getScale();

			Apex::SceneGraph::SceneNode* node = obj->GetSceneNode().get();

			size_t parentId = SIZE_MAX;

			if (node->GetParent())
			{
				parentId = node->GetParent()->GetObject()->GetId();
			}

			file << "  {\n";
			file << "    \"id\": " << obj->GetId() << ",\n";
			file << "    \"parent\": " << (parentId == SIZE_MAX ? "null" : std::to_string(parentId)) << ",\n";
			file << "    \"name\": \"" << obj->GetName() << "\",\n";
			file << "    \"position\": " << pos << ",\n";
			file << "    \"rotation\": [" << rot[0] << "," << rot[1] << "," << rot[2] << "," << rot[3] << "],\n";
			file << "    \"scale\": " << scale << ",\n";

			file << "    \"components\": {\n";

			bool firstComponent = true;

			for (auto* component : obj->GetComponents())
			{
				if (!firstComponent)
					file << ",\n";

				file << "      \"" << component->GetTypeName() << "\": {\n";
				file << "        \"enabled\": " << (component->IsEnabled() ? "true" : "false") << ",\n";
				component->Serialize(file);
				file << "      }";

				firstComponent = false;
			}

			file << "\n    }\n";
			file << "  }";

			if (obj != m_objects.back())
				file << ",";

			file << "\n";
		}

		file << "]\n";
		file << "}\n";

		LOG_INFO("Scene saved to {}", m_name);

		ClearDirty();
	}

	void Scene::SetOnMarkedDirtyCallback(std::function<void()> callback)
	{
		if (callback)
			m_onMarkedDirty = callback;
	}

	void Scene::CloneObjects(std::unique_ptr<Scene>& copy) const
	{
		for (const auto& obj : m_objects)
		{
			copy->m_objects.push_back(obj->Clone());
			std::unique_ptr<Object>& newObj = copy->m_objects.back();

			RigidBodyComponent* body = newObj->GetComponent<RigidBodyComponent>();
			Collider* collider = newObj->GetComponent<Collider>();

			if (body && collider)
			{
				copy->m_physics.CreateActor(*collider, *body);
			}

			newObj->SetOnChangedCallback([this]()
				{
					MarkDirty();
				});
		}
	}

	void Scene::ReparentObjects(std::unique_ptr<Scene>& copy) const
	{
		for (const auto& obj : m_objects)
		{
			for (auto const& child : obj->GetSceneNode()->GetChildren())
			{
				size_t parentId = obj->GetId();
				size_t childId = child->GetObject()->GetId();

				auto parentIt = std::find_if(copy->m_objects.begin(), copy->m_objects.end(),
					[parentId](const auto& o) { return o->GetId() == parentId; });
				auto childIt = std::find_if(copy->m_objects.begin(), copy->m_objects.end(),
					[childId](const auto& o) { return o->GetId() == childId; });

				if (parentIt != copy->m_objects.end() && childIt != copy->m_objects.end())
					(*parentIt)->GetSceneNode()->AddChild((*childIt)->GetSceneNode());
			}
		}
	}
}