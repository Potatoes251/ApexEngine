#include "Scene.h"
#include "SceneGraph.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "../../Physic/Header/Rigidbody.h"
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
	
	void Scene::Update(float deltaTime)
	{
		for (auto& obj : m_objects)
			obj->Update(deltaTime);
	}

	void Scene::Render(const LibMath::Matrix4& view, const LibMath::Matrix4& projection, const LibMath::Vector3& cameraPosition)
	{
		for (auto& obj : m_objects)
		{
			MeshRenderer* renderer = obj->GetComponent<MeshRenderer>();
			if (!renderer)
				continue;

			renderer->Render(view, projection, cameraPosition);
		}
	}

	void Scene::Save(const std::string& path)
	{
		std::ofstream file(path);

		if (!file)
		{
			std::cout << "File failed to open\n";
		}

		if (!file.is_open())
		{
			std::cout << "Failed to save scene\n";
			return;
		}

		file << std::fixed;
		file << std::setprecision(6);

		file << "{\n";
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
			file << "    \"parent\": " << parentId << ",\n";
			file << "    \"position\": [" << pos[0] << "," << pos[1] << "," << pos[2] << "],\n";
			file << "    \"rotation\": [" << rot[0] << "," << rot[1] << "," << rot[2] << "],\n";
			file << "    \"scale\": [" << scale[0] << "," << scale[1] << "," << scale[2] << "],\n";

			file << "    \"components\": {\n";

			bool first = true;

			for (auto* component : obj->GetComponents())
			{
				if (!first)
					file << ",\n";

				component->Serialize(file);
				first = false;
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