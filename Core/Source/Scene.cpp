#include "Scene.h"
#include "SceneGraph.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

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
} // namespace Apex::Rendering