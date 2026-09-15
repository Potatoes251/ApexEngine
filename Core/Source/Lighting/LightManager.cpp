#include "Lighting/LightManager.h"

#include "Lighting/Lights.h"

#include "Object.h"

#include "RHI.h"

#include "LibMath/Trigonometry.h"

#include <cassert>

using namespace Apex::Lighting;

LightManager::LightManager(Rendering::IRHI* rhi)
{
	m_rhi = rhi;

	assert(m_rhi != nullptr && "Lights cannot work without the rhi");

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
	bufferP.m_count[0] = (float)std::min(lights.m_pointLights.size(), MAX_POINT_LIGHTS);
	for (int i = 0; i < bufferP.m_count[0]; i++)
	{
		PointLightComponent* light = lights.m_pointLights[i];
		light->FillGpuData(&bufferP.m_lights[i]);
	}
	m_rhi->BindUniformBuffer(m_pointLightBuffer, sizeof(GPUPointLightBuffer), &bufferP);

	GPUSpotLightBuffer bufferS;
	bufferS.m_count[0] = (float)std::min(lights.m_spotLights.size(), MAX_DIR_LIGHTS);
	for (int i = 0; i < bufferS.m_count[0]; i++)
	{
		SpotLightComponent* light = lights.m_spotLights[i];
		light->FillGpuData(&bufferS.m_lights[i]);
	}
	m_rhi->BindUniformBuffer(m_spotLightBuffer, sizeof(GPUSpotLightBuffer), &bufferS);

	GPUDirectionalLightBuffer bufferD;
	bufferD.m_count[0] = (float)std::min(lights.m_directionalLights.size(), MAX_SPOT_LIGHTS);
	for (int i = 0; i < bufferD.m_count[0]; i++)
	{
		DirectionalLightComponent* light = lights.m_directionalLights[i];
		light->FillGpuData(&bufferD.m_lights[i]);
	}
	m_rhi->BindUniformBuffer(m_directionalLightBuffer, sizeof(GPUDirectionalLightBuffer), &bufferD);
}
