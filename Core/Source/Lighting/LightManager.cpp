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

	m_pointLightBuffer = rhi->CreateUniformBuffer(0, sizeof(GPUPointLight) * m_maxPointLights);
	m_spotLightBuffer = rhi->CreateUniformBuffer(1, sizeof(GPUSpotLight) * m_maxSpotLights);
	m_directionalLightBuffer = rhi->CreateUniformBuffer(2, sizeof(GPUDirectionalLight) * m_maxDirectionalLights);

	
	m_gpuPointLights.reserve(m_maxPointLights);
	m_gpuSpotLights.reserve(m_maxSpotLights);
	m_gpuDirectionalLights.reserve(m_maxDirectionalLights);
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
	m_gpuPointLights.clear();
	m_gpuSpotLights.clear();
	m_gpuDirectionalLights.clear();

	for (PointLightComponent* light : lights.m_pointLights)
	{
		GPUPointLight gpuLight =
		{
			.m_position = light->GetOwner()->GetGlobalTransform().getPosition(),
			.m_color = light->GetColor(),
			.m_intensity = light->GetIntensity(),
			.m_radius = light->GetRadius()
		};

		m_gpuPointLights.push_back(gpuLight);
	}
	size_t nbLights = std::min(m_gpuPointLights.size(), m_maxPointLights);
	m_rhi->BindUniformBuffer(m_pointLightBuffer, sizeof(GPUPointLight) * nbLights, m_gpuPointLights.data());

	for (SpotLightComponent* light : lights.m_spotLights)
	{
		GPUSpotLight gpuLight =
		{
			.m_position = light->GetOwner()->GetGlobalTransform().getPosition(),
			.m_direction = light->GetOwner()->GetGlobalTransform().getForward(),
			.m_color = light->GetColor(),
			.m_intensity = light->GetIntensity(),
			.m_innerCutoff = light->GetInnerCutoff(),
			.m_outerCutoff = light->GetOuterCutoff()
		};
		m_gpuSpotLights.push_back(gpuLight);
	}
	nbLights = std::min(m_gpuSpotLights.size(), m_maxSpotLights);
	m_rhi->BindUniformBuffer(m_spotLightBuffer, sizeof(GPUSpotLight) * nbLights, m_gpuSpotLights.data());

	for (DirectionalLightComponent* light : lights.m_directionalLights)
	{
		GPUDirectionalLight gpuLight =
		{
			.m_direction = light->GetOwner()->GetGlobalTransform().getForward(),
			.m_color = light->GetColor(),
			.m_intensity = light->GetIntensity()
		};

		m_gpuDirectionalLights.push_back(gpuLight);
	}
	nbLights = std::min(m_gpuDirectionalLights.size(), m_maxDirectionalLights);
	m_rhi->BindUniformBuffer(m_directionalLightBuffer, sizeof(GPUDirectionalLight) * nbLights, m_gpuDirectionalLights.data());
}
