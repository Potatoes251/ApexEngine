#include "Lighting/ShadowManager.h"

#include "Lighting/VisibleLights.h"

#include "Lighting/Lights.h"

#include <cassert>

using namespace Apex::Lighting;

ShadowManager::ShadowManager(Rendering::IRHI* rhi) : m_rhi(rhi)
{
	assert(rhi != nullptr && "Shadows cannot work withput the rhi");

	ShadowMap shadow;
	shadow.m_shadowTexture = rhi->CreateShadowMap(m_shadowWidth, m_shadowHeight);

	m_shadowFbo = rhi->CreateEmptyFrameBuffer(shadow.m_shadowTexture, m_shadowWidth, m_shadowHeight);

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

void ShadowManager::RenderShadows(VisibleLights const& lights)
{
	for (DirectionalLightComponent* light : lights.m_directionalLights)
	{
		if (!light->IsCastingShadow()) continue;
	}
}
