#include "Scene.h"
#include "SceneGraph.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "Camera.h"

#include "Rigidbody.h"
#include "BoxCollider.h"

#include "Lighting/Lights.h"
#include "Controller/PlayerController.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace Apex::Physic;
using namespace Apex::Data;

namespace Apex::Rendering
{
	std::unique_ptr<Scene> Scene::Clone()
	{
		std::unique_ptr<Scene> copy = std::make_unique<Scene>();
		copy->m_skybox = std::make_unique<Skybox>(*m_skybox);

		copy->m_mainCamera = m_mainCamera;

		for (const auto& obj : m_objects)
		{
			copy->m_objects.push_back(obj->Clone());
			std::unique_ptr<Object>& newObj = copy->m_objects[copy->m_objects.size() - 1];

			RigidBodyComponent* body = newObj->GetComponent<RigidBodyComponent>();
			Collider* collider = newObj->GetComponent<Collider>();

			if (body && collider)
			{
				PhysMatHandle mat = collider->GetMaterial();

				if (mat.IsValid())
					collider->SetMaterial(copy->m_physics.CreateMaterial(
						mat->GetPath(), mat->GetStaticFriction(), mat->GetDynamicFriction(), mat->GetBounciness()));
				
				copy->m_physics.CreateActor(*collider, *body);
			}
		}

		return copy;
	}

	Lighting::VisibleLights Scene::GetVisibleLights()
	{
		Lighting::VisibleLights visibleLights;

		for (std::unique_ptr<Object>& obj : m_objects)
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

		m_objects.push_back(std::move(obj));
		return ptr;
	}

	const std::vector<std::unique_ptr<Object>>& Scene::GetObjects() const
	{
		return m_objects;
	}

	void Scene::DestroyObject(Object* obj)
	{
		if (!obj)
			return;

		std::shared_ptr<Apex::SceneGraph::SceneNode> node = obj->GetSceneNode();

		if (node && node->GetParent())
			node->GetParent()->DestroyChild(node);

		auto it = std::find_if(
			m_objects.begin(),
			m_objects.end(),
			[&](const std::unique_ptr<Object>& ptr)
			{
				return ptr.get() == obj;
			});

		if (it != m_objects.end())
			m_objects.erase(it);
	}
		
	void Scene::Start()
	{
		for (auto& obj : m_objects)
		{
			obj->Start();

			if (m_mainCamera) continue;

			Controller::PlayerController* player = obj->GetComponent<Controller::PlayerController>();
			if (player)
			{
				m_mainCamera = obj->GetComponent<Camera>();
			}
		}
	}

	void Scene::Update(float deltaTime_s)
	{
		for (auto& obj : m_objects)
			obj->Update(deltaTime_s);

		if (m_mainCamera)
		{
			m_mainCamera->SetPosition(m_mainCamera->GetOwner()->GetGlobalTransform().getPosition());
		}
	}

	void Scene::FixedUpdate(float deltaTime_s)
	{
		m_physics.Update(deltaTime_s);
	}

	void Scene::Render(RenderPassInfo passInfo)
	{
		for (auto& obj : m_objects)
		{
			passInfo.m_id++;
			MeshRenderer* renderer = obj->GetComponent<MeshRenderer>();
			if (!renderer)
				continue;
			renderer->Render(passInfo);
		}
		if (passInfo.m_flags & RenderSkybox && passInfo.m_camera)
		{
			LibMath::Matrix4 view = passInfo.m_camera->GetViewMatrix();
			view[3][0] = 0;
			view[3][1] = 0;
			view[3][2] = 0;
			m_skybox->Render(passInfo.m_camera->GetProjection() * view);
		}
		if (passInfo.m_flags & ShowCollider)
			m_physics.DrawColliders();
	}

	void Scene::InsertBefore(Object* dragged, Object* target)
	{
		if (!dragged || !target || dragged == target)
			return;

		auto& objs = m_objects;

		auto itDragged = std::find_if(objs.begin(), objs.end(),
			[&](auto& o) { return o.get() == dragged; });

		auto itTarget = std::find_if(objs.begin(), objs.end(),
			[&](auto& o) { return o.get() == target; });

		if (itDragged == objs.end() || itTarget == objs.end())
			return;

		if (itDragged < itTarget)
			std::rotate(itDragged, itDragged + 1, itTarget);
		else
			std::rotate(itTarget, itDragged, itDragged + 1);
	}

	void Scene::InsertAfter(Object* dragged, Object* target)
	{
		if (!dragged || !target || dragged == target)
			return;

		auto& objs = m_objects;

		auto itDragged = std::find_if(objs.begin(), objs.end(),
			[&](auto& o) { return o.get() == dragged; });

		auto itTarget = std::find_if(objs.begin(), objs.end(),
			[&](auto& o) { return o.get() == target; });

		if (itDragged == objs.end() || itTarget == objs.end())
			return;

		if (itDragged < itTarget)
			std::rotate(itDragged, itDragged + 1, itTarget + 1);
		else
			std::rotate(itTarget + 1, itDragged, itDragged + 1);
	}

	void Scene::Save(const std::string& path)
	{
		std::ofstream file(path);

		if (!file)
		{
			std::cout << "Failed to open file\n";
			return;
		}

		file << std::fixed;
		file << std::setprecision(6);

		file << "{\n";

		file << "\"materials\": [\n";

		auto& materials = m_physics.GetMaterials();

		bool first = true;

		for (auto const& [name, entry] : materials)
		{
			auto* material = dynamic_cast<PhysicMaterial*>(entry.m_resource.get());
			if (!material)
				continue;

			if (!first)
				file << ",\n";

			first = false;

			file <<
				"  {\n"
				"    \"id\": " << material->GetId() << ",\n"
				"    \"static friction\": " << material->GetStaticFriction() << ",\n"
				"    \"dynamic friction\": " << material->GetDynamicFriction() << ",\n"
				"    \"bounciness\": " << material->GetBounciness() << "\n"
				"  }";
		}

		file << "\n],\n";

		file << "\"objects\": [\n";

		for (size_t i = 0; i < m_objects.size(); i++)
		{
			auto& obj = m_objects[i];

			const auto& transform = obj->GetLocalTransform();

			auto pos = transform.getPosition();
			auto rot = transform.getRotation();
			auto scale = transform.getScale();

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
			file << "    \"position\": [" << pos[0] << "," << pos[1] << "," << pos[2] << "],\n";
			file << "    \"rotation\": [" << rot[0] << "," << rot[1] << "," << rot[2] << "," << rot[3] << "],\n";
			file << "    \"scale\": [" << scale[0] << "," << scale[1] << "," << scale[2] << "],\n";

			file << "    \"components\": {\n";

			bool firstComponent = true;

			for (auto* component : obj->GetComponents())
			{
				if (!firstComponent)
					file << ",\n";

				component->Serialize(file);
				firstComponent = false;
			}

			file << "\n    }\n";
			file << "  }";

			if (i < m_objects.size() - 1)
				file << ",";

			file << "\n";
		}

		file << "]\n";
		file << "}\n";

		std::cout << "Scene saved to " << path << std::endl;
	}
}