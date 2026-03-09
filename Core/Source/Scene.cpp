#include "Scene.h"
#include "SceneGraph.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

namespace Apex::Rendering
{
	Scene::Scene(IRHI* rhi, Apex::Core::ThreadPool& threadPool) : m_rhi(rhi), m_resourceManager(threadPool) {}

	Apex::Data::Object* Scene::AddMesh(const std::string& meshFile, const std::string& textureFile, const std::string& shaderName)
	{
		MeshRenderer renderer;

		renderer.m_mesh = m_resourceManager.CreateAsync<Mesh>(meshFile);
		renderer.m_texture = m_resourceManager.CreateAsync<Texture>(textureFile);
		renderer.m_shader = m_resourceManager.CreateAsync<Shader>(shaderName);

		Apex::Data::Object* obj = renderer.object.get();
		m_renderers.push_back(std::move(renderer));
		return obj;
	}
	void Scene::Update(const LibMath::Matrix4& view, const LibMath::Matrix4& projection,
					   const LibMath::Vector3& cameraPosition, float deltaTime)
	{
		m_resourceManager.ProcessGpuUploads();

		for (MeshRenderer& renderer : m_renderers)
		{
			if (!renderer.m_mesh.IsReady() || !renderer.m_shader.IsReady() || !renderer.m_texture.IsReady())
				continue;

			renderer.m_shader->Use();

			LibMath::Matrix4 model = renderer.object->GetGlobalTransform();

			renderer.m_shader->SetUniform("uModel", model);
			renderer.m_shader->SetUniform("uView", view);
			renderer.m_shader->SetUniform("uProjection", projection);
										 
			renderer.m_shader->SetUniform("uLightPos", { 2.f, 3.f, 2.f });
			renderer.m_shader->SetUniform("uLightColor", { 1.f, 1.f, 1.f });
			renderer.m_shader->SetUniform("uViewPos", cameraPosition);

			renderer.m_texture->Bind(0);
			renderer.m_shader->SetUniform("uTexture", 0);
			renderer.m_shader->SetUniform("uUseTexture", 1);

			renderer.m_mesh->Draw();
		}
	}
} // namespace Apex::Rendering