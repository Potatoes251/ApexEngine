#include "Lighting/ShadowManager.h"

#include "Lighting/VisibleLights.h"
#include "Lighting/Lights.h"

#include "Scene.h"

#include "FirstPersonCamera.h"
#include "ThirdPersonCamera.h"

#include <cassert>

using namespace Apex::Lighting;
using namespace Apex::Rendering;

ShadowManager::ShadowManager(IRHI* rhi, Resources::ResourceHandle<Shader> dirShader, Resources::ResourceHandle<Shader> omniShader) 
	: m_rhi(rhi), m_dirShader(dirShader), m_omniShader(omniShader)
{
	assert(rhi != nullptr && "Shadows cannot work withput the rhi");

	ShadowMap shadow;
	shadow.m_shadowTexture = rhi->CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

	m_shadowFbo = rhi->CreateEmptyFrameBuffer(shadow.m_shadowTexture, SHADOW_WIDTH, SHADOW_HEIGHT);

	m_shadowMaps.push_back(shadow);
}

ShadowManager::~ShadowManager()
{
	if (m_rhi)
	{
		for (ShadowMap& shadow : m_shadowMaps)
		{
			m_rhi->DeleteTexture(shadow.m_shadowTexture);
		}
		m_rhi->DeleteFrameBuffer(m_shadowFbo);
	}
}

void ShadowManager::RenderShadows(VisibleLights const& lights, Scene* scene, Camera* camera)
{
	m_rhi->BindFrameBuffer(m_shadowFbo, SHADOW_WIDTH, SHADOW_HEIGHT);
	m_rhi->SetDepthTest(true);

	RenderPassInfo pass;
	pass.m_flags |= ShadowPass;
	pass.m_flags |= IgnoreModelShader;
	pass.m_flags &= ~RenderSkybox;
	pass.m_shader = m_dirShader;
	m_dirShader->Use();
	for (DirectionalLightComponent* light : lights.m_directionalLights)
	{
		if (!light->IsCastingShadow()) continue;

		if (light->GetShadowIdx() == -1)	light->SetShadowIdx(AcquireShadowMap(light, camera));

		if (light->GetShadowIdx() == -1) continue;

		ShadowMap& shadow = m_shadowMaps[light->GetShadowIdx()];

		shadow.m_viewProj = light->GetProjMatrix() * light->GetViewMatrix(camera)[0];

		m_rhi->AttachDepthTexture(shadow.m_shadowTexture);
		m_rhi->ClearDepth();

		m_dirShader->SetUniform("lightSpaceMatrix", shadow.m_viewProj);

		scene->Render(pass);
	}
	for (SpotLightComponent* light : lights.m_spotLights)
	{
		if (!light->IsCastingShadow()) continue;

		if (light->GetShadowIdx() == -1)	light->SetShadowIdx(AcquireShadowMap(light, camera));

		if (light->GetShadowIdx() == -1) continue;

		ShadowMap& shadow = m_shadowMaps[light->GetShadowIdx()];

		shadow.m_viewProj = light->GetProjMatrix() * light->GetViewMatrix(camera)[0];

		m_rhi->AttachDepthTexture(shadow.m_shadowTexture);
		m_rhi->ClearDepth();

		m_dirShader->SetUniform("lightSpaceMatrix", shadow.m_viewProj);

		scene->Render(pass);
	}

	pass.m_shader = m_omniShader;
	m_omniShader->Use();
	for (PointLightComponent* light : lights.m_pointLights)
	{
		if (!light->IsCastingShadow()) continue;

		if (light->GetShadowIdx() == -1)	light->SetShadowIdx(AcquireShadowCubeMap(light));

		if (light->GetShadowIdx() == -1) continue;

		ShadowCubeMap& shadow = m_shadowCubeMaps[light->GetShadowIdx()];

		LibMath::Matrix4 proj = light->GetProjMatrix();
		std::vector<LibMath::Matrix4> views = light->GetViewMatrix(nullptr);

		assert(views.size() == 6 && "incorrect amount of view matrices for a cubemap");

		for (int i = 0; i < 6; i++)
		{
			shadow.m_viewProj[i] = proj * views[i];
		}

		m_rhi->AttachDepthCubeTexture(shadow.m_shadowTexture);
		m_rhi->ClearDepth();

		m_omniShader->SetUniform("shadowMatrices", shadow.m_viewProj[0], 6);
		m_omniShader->SetUniform("lightPos", light->GetOwner()->GetGlobalTransform().getPosition());
		m_omniShader->SetUniform("farPlane", light->GetFarPlane());

		scene->Render(pass);
	}

	m_rhi->UnBindFrameBuffer();
}

void ShadowManager::BindShadowMaps() const
{
	for (int i = 0; i < m_shadowMaps.size(); i++)
	{
		ShadowMap const& shadow = m_shadowMaps[i];

		if (shadow.m_ownerLightId != -1)
		{
			int textureUnit = FIRST_SHADOW_2D_SLOT + i;
			m_rhi->BindTexture(shadow.m_shadowTexture, textureUnit);
		}
	}

	for (int i = 0; i < m_shadowCubeMaps.size(); i++)
	{
		ShadowCubeMap const& shadow = m_shadowCubeMaps[i];

		if (shadow.m_ownerLightId != -1)
		{
			int textureUnit = FIRST_SHADOW_CUBE_SLOT + i;
			m_rhi->BindTexture(shadow.m_shadowTexture, textureUnit);
		}
	}
}

int ShadowManager::AcquireShadowMap(LightComponent* light, Camera* camera)
{
	for (int i = 0; i < m_shadowMaps.size(); i++)
	{
		ShadowMap& shadow = m_shadowMaps[i];

		if (shadow.m_ownerLightId == -1) 
		{
			shadow.m_ownerLightId = light->GetLightId();
			shadow.m_viewProj = light->GetProjMatrix() * light->GetViewMatrix(camera)[0];
			return i;
		}
	}

	if (m_shadowMaps.size() >= MAX_SHADOW_2D)
		return -1;

	ShadowMap shadow;
	shadow.m_ownerLightId = light->GetLightId();
	shadow.m_shadowTexture = m_rhi->CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);
	shadow.m_viewProj = light->GetProjMatrix() * light->GetViewMatrix(camera)[0];

	m_shadowMaps.push_back(shadow);

	return m_shadowMaps.size() - 1;
}

int ShadowManager::AcquireShadowCubeMap(LightComponent* light)
{
	for (int i = 0; i < m_shadowCubeMaps.size(); i++)
	{
		ShadowCubeMap& shadow = m_shadowCubeMaps[i];

		if (shadow.m_ownerLightId == -1)
		{
			shadow.m_ownerLightId = light->GetLightId();
			LibMath::Matrix4 proj = light->GetProjMatrix();
			std::vector<LibMath::Matrix4> views = light->GetViewMatrix(nullptr);

			assert(views.size() == 6 && "incorrect amount of view matrices for a cubemap");

			for (int i = 0; i < 6; i++)
			{
				shadow.m_viewProj[i] = proj * views[i];
			}
			return i;
		}
	}

	if (m_shadowCubeMaps.size() >= MAX_SHADOW_CUBE)
		return -1;

	ShadowCubeMap shadow;
	shadow.m_ownerLightId = light->GetLightId();
	shadow.m_shadowTexture = m_rhi->CreateShadowCubeMap(SHADOW_WIDTH, SHADOW_HEIGHT);

	LibMath::Matrix4 proj = light->GetProjMatrix();
	std::vector<LibMath::Matrix4> views = light->GetViewMatrix(nullptr);

	assert(views.size() == 6 && "incorrect amount of view matrices for a cubemap");

	for (int i = 0; i < 6; i++)
	{
		shadow.m_viewProj[i] = proj * views[i];
	}

	m_shadowCubeMaps.push_back(shadow);

	return m_shadowCubeMaps.size() - 1;
}
