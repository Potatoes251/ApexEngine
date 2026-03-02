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

			auto* mesh = static_cast<Mesh*>   (renderer.m_mesh.operator->());
			auto* shader = static_cast<Shader*> (renderer.m_shader.operator->());
			auto* texture = static_cast<Texture*>(renderer.m_texture.operator->());

			shader->Use();
			GLuint prog = shader->GetProgram();

			LibMath::Matrix4 model = renderer.object->GetNode()->GetWorldTransform();

			m_rhi->SetUniformMat4(prog, "uModel", model);
			m_rhi->SetUniformMat4(prog, "uView", view);
			m_rhi->SetUniformMat4(prog, "uProjection", projection);

			m_rhi->SetUniformVec3(prog, "uLightPos", { 2.f, 3.f, 2.f });
			m_rhi->SetUniformVec3(prog, "uLightColor", { 1.f, 1.f, 1.f });
			m_rhi->SetUniformVec3(prog, "uViewPos", cameraPosition);

			texture->Bind(0);
			m_rhi->SetUniformInt(prog, "uTexture", 0);
			m_rhi->SetUniformInt(prog, "uUseTexture", 1);

			mesh->Draw();
		}
	}
} // namespace Apex::Rendering