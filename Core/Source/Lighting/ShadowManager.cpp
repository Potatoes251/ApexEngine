#include "Lighting/ShadowManager.h"

#include "Lighting/VisibleLights.h"
#include "Lighting/Lights.h"

#include "Scene.h"

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

void ShadowManager::RenderShadows(VisibleLights const& lights, Scene* scene)
{
	m_rhi->BindFrameBuffer(m_shadowFbo, SHADOW_WIDTH, SHADOW_HEIGHT);
	
	RenderPassInfo pass;
	pass.m_flags |= ShadowPass;
	pass.m_flags |= IgnoreModelShader;
	pass.m_flags &= ~RenderSkybox;
	pass.m_shader = m_dirShader;
	
	for (DirectionalLightComponent* light : lights.m_directionalLights)
	{
		if (!light->IsCastingShadow()) continue;

		if (light->GetShadowIdx() == -1)	light->SetShadowIdx(AcquireShadowMap(light));

		if (light->GetShadowIdx() == -1) continue;

		m_rhi->AttachDepthTexture(m_shadowMaps[light->GetShadowIdx()].m_shadowTexture);
		m_rhi->ClearDepth();

		scene->Render(pass);
	}
	for (SpotLightComponent* light : lights.m_spotLights)
	{
		if (!light->IsCastingShadow()) continue;

		if (light->GetShadowIdx() == -1)	light->SetShadowIdx(AcquireShadowMap(light));

		if (light->GetShadowIdx() == -1) continue;

		m_rhi->AttachDepthTexture(m_shadowMaps[light->GetShadowIdx()].m_shadowTexture);
		m_rhi->ClearDepth();

		scene->Render(pass);
	}

	pass.m_shader = m_omniShader;
	for (PointLightComponent* light : lights.m_pointLights)
	{
		if (!light->IsCastingShadow()) continue;

		if (light->GetShadowIdx() == -1)	light->SetShadowIdx(AcquireShadowCubeMap(light));

		if (light->GetShadowIdx() == -1) continue;

		m_rhi->AttachDepthCubeTexture(m_shadowCubeMaps[light->GetShadowIdx()].m_shadowTexture);
		m_rhi->ClearDepth();

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
			//m_shader->SetUniform(("shadowMap[" + std::to_string(i) + "]").c_str(), textureUnit);
		}
	}

	for (int i = 0; i < m_shadowCubeMaps.size(); i++)
	{
		ShadowCubeMap const& shadow = m_shadowCubeMaps[i];

		if (shadow.m_ownerLightId != -1)
		{
			int textureUnit = FIRST_SHADOW_CUBE_SLOT + i;
			m_rhi->BindTexture(shadow.m_shadowTexture, textureUnit);
			//m_shader->SetUniform(("shadowCubeMap[" + std::to_string(i) + "]").c_str(), textureUnit);
		}
	}
}

int ShadowManager::AcquireShadowMap(LightComponent* light)
{
	for (int i = 0; i < m_shadowMaps.size(); i++)
	{
		ShadowMap& shadow = m_shadowMaps[i];

		if (shadow.m_ownerLightId == -1) 
		{
			shadow.m_ownerLightId = light->GetLightId();
			return i;
		}
	}

	if (m_shadowMaps.size() >= MAX_SHADOW_2D)
		return -1;

	ShadowMap shadow;
	shadow.m_ownerLightId = light->GetLightId();
	shadow.m_shadowTexture = m_rhi->CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

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
			return i;
		}
	}

	if (m_shadowCubeMaps.size() >= MAX_SHADOW_CUBE)
		return -1;

	ShadowCubeMap shadow;
	shadow.m_ownerLightId = light->GetLightId();
	shadow.m_shadowTexture = m_rhi->CreateShadowCubeMap(SHADOW_WIDTH, SHADOW_HEIGHT);

	m_shadowCubeMaps.push_back(shadow);

	return m_shadowCubeMaps.size() - 1;
}
