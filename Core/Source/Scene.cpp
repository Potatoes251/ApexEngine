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
}