#include "Lighting/LightManager.h"

#include "Lighting/PointLightComponent.h"
#include "Lighting/SpotLightComponent.h"
#include "Lighting/DirectionalLightComponent.h"

#include "Object.h"

#include <cassert>

using namespace Apex::Lighting;

LightManager::LightManager(Rendering::IRHI* rhi)
{
	m_rhi = rhi;

	assert(m_rhi != nullptr && "the rhi shouldn't be nullptr");

	m_pointLightBuffer = rhi->CreateUniformBuffer(0, sizeof(GPUPointLightBuffer));
	m_directionalLightBuffer = rhi->CreateUniformBuffer(1, sizeof(GPUDirectionalLightBuffer));
	m_spotLightBuffer = rhi->CreateUniformBuffer(2, sizeof(GPUSpotLightBuffer));
}

LightManager::~LightManager()
{
	if (m_rhi)
	{
		m_rhi->DeleteUniformBuffer(m_pointLightBuffer);
		m_rhi->DeleteUniformBuffer(m_spotLightBuffer);
		m_rhi->DeleteUniformBuffer(m_directionalLightBuffer);
	}
}

void LightManager::UploadToGpu(VisibleLights const& lights)
{
	GPUPointLightBuffer bufferP;
	bufferP.m_count[0] = std::min(lights.m_pointLights.size(), m_maxPointLights);
	for (int i = 0; i < bufferP.m_count[0]; i++)
	{
		PointLightComponent* light = lights.m_pointLights[i];
		bufferP.m_lights[i] =
		{
			.m_position = { light->GetOwner()->GetGlobalTransform().getPosition(), 0 },
			.m_color = { light->GetColor(), 0 },
			.m_param = { light->GetIntensity(), light->GetRadius(), 0, 0 }
		};
	}
	m_rhi->BindUniformBuffer(m_pointLightBuffer, sizeof(GPUPointLightBuffer), &bufferP);

	GPUSpotLightBuffer bufferS;
	bufferS.m_count[0] = std::min(lights.m_spotLights.size(), m_maxSpotLights);
	for (int i = 0; i < bufferS.m_count[0]; i++)
	{
		SpotLightComponent* light = lights.m_spotLights[i];
		bufferS.m_lights[i] =
		{
			.m_position = { light->GetOwner()->GetGlobalTransform().getPosition(), 0 },
			.m_direction = { light->GetOwner()->GetGlobalTransform().getForward(),0 },
			.m_color = { light->GetColor(), 0 },
			.m_param = { light->GetIntensity(), light->GetInnerCutoff(), light->GetOuterCutoff(), 0 }
		};
	}
	m_rhi->BindUniformBuffer(m_spotLightBuffer, sizeof(GPUSpotLightBuffer), &bufferS);

	GPUDirectionalLightBuffer bufferD;
	bufferD.m_count[0] = std::min(lights.m_directionalLights.size(), m_maxDirectionalLights);
	for (int i = 0; i < bufferD.m_count[0]; i++)
	{
		DirectionalLightComponent* light = lights.m_directionalLights[i];
		bufferD.m_lights[i] =
		{
			.m_direction = { light->GetOwner()->GetGlobalTransform().getForward(),0 },
			.m_color = { light->GetColor(), 0 },
			.m_param = { light->GetIntensity(), 0, 0, 0 }
		};
	}
	m_rhi->BindUniformBuffer(m_directionalLightBuffer, sizeof(GPUDirectionalLightBuffer), &bufferD);
}
