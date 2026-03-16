#include "Scene.h"
#include "SceneGraph.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "Rigidbody.h"
#include "BoxCollider.h"

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace Apex::Physic;

namespace Apex::Rendering
{
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
	
	void Scene::Update(float deltaTime)
	{
		for (auto& obj : m_objects)
			obj->Update(deltaTime);
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
	}

	void Scene::Save(const std::string& path, Apex::Physic::PhysicSystem& phys)
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

		auto& materials = phys.GetMaterials();

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