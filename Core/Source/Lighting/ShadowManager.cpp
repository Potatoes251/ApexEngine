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

	m_lightMatrixBuffer = rhi->CreateUniformBuffer(3, sizeof(LibMath::Matrix4) * 10);
}

ShadowManager::~ShadowManager()
{
	if (m_rhi)
	{
		m_rhi->DeleteFrameBuffer(m_shadowFbo);

		for (ShadowMap& shadow : m_shadowMaps)
		{
			m_rhi->DeleteTexture(shadow.m_shadowTexture);
		}

		for (ShadowCubeMap& shadowCube : m_shadowCubeMaps)
		{
			m_rhi->DeleteTexture(shadowCube.m_shadowTexture);
		}

		m_rhi->DeleteUniformBuffer(m_lightMatrixBuffer);
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

	int shadowCount = 0;
	for (DirectionalLightComponent* light : lights.m_directionalLights)
	{
		if (!light->IsCastingShadow() || !AcquireShadowMap(light, camera, shadowCount)) continue;

		ShadowMap& shadow = m_shadowMaps[shadowCount];

		m_rhi->AttachDepthTexture(shadow.m_shadowTexture);
		m_rhi->ClearDepth();

		m_dirShader->SetUniform("lightSpaceMatrix", shadow.m_viewProj);

		scene->Render(pass);

		shadowCount++;
	}
	for (SpotLightComponent* light : lights.m_spotLights)
	{
		if (!light->IsCastingShadow() || !AcquireShadowMap(light, camera, shadowCount)) continue;

		ShadowMap& shadow = m_shadowMaps[shadowCount];

		m_rhi->AttachDepthTexture(shadow.m_shadowTexture);
		m_rhi->ClearDepth();

		m_dirShader->SetUniform("lightSpaceMatrix", shadow.m_viewProj);

		scene->Render(pass);

		shadowCount++;
	}
	shadowCount = 0;
	pass.m_shader = m_omniShader;
	m_omniShader->Use();
	for (PointLightComponent* light : lights.m_pointLights)
	{
		if (!light->IsCastingShadow() || !AcquireShadowCubeMap(light, shadowCount)) continue;

		ShadowCubeMap& shadow = m_shadowCubeMaps[shadowCount];

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

		shadowCount++;
	}

	m_rhi->UnBindFrameBuffer();
}

void ShadowManager::BindShadowMaps()
{
	LibMath::Matrix4 lightSpaceMatrix[10];
	for (int i = 0; i < m_shadowMaps.size(); i++)
	{
		ShadowMap& shadow = m_shadowMaps[i];

		int textureUnit = FIRST_SHADOW_2D_SLOT + i;
		m_rhi->BindTexture(shadow.m_shadowTexture, textureUnit);

		lightSpaceMatrix[i] = shadow.m_viewProj;
	}

	m_rhi->BindUniformBuffer(m_lightMatrixBuffer, sizeof(LibMath::Matrix4) * 10, &lightSpaceMatrix);

	for (int i = 0; i < m_shadowCubeMaps.size(); i++)
	{
		ShadowCubeMap& shadow = m_shadowCubeMaps[i];

		int textureUnit = FIRST_SHADOW_CUBE_SLOT + i;
		m_rhi->BindTextureCube(shadow.m_shadowTexture, textureUnit);
	}
}

bool ShadowManager::AcquireShadowMap(LightComponent* light, Camera* camera, int idx)
{
	if (idx < 0 || idx >= MAX_SHADOW_2D) return false;

	if (idx < m_shadowMaps.size())
	{
		m_shadowMaps[idx].m_viewProj = light->GetProjMatrix() * light->GetViewMatrix(camera)[0];
		light->SetShadowIdx(idx);
		return true;
	}

	ShadowMap shadow;
	shadow.m_shadowTexture = m_rhi->CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);
	shadow.m_viewProj = light->GetProjMatrix() * light->GetViewMatrix(camera)[0];

	m_shadowMaps.push_back(shadow);
	light->SetShadowIdx(m_shadowMaps.size() - 1);

	return true;
}

bool ShadowManager::AcquireShadowCubeMap(LightComponent* light, int idx)
{
	if (idx < 0 || idx >= MAX_SHADOW_CUBE) return false;

	if (idx < m_shadowCubeMaps.size())
	{
		ShadowCubeMap& shadow = m_shadowCubeMaps[idx];

		light->SetShadowIdx(idx);
		LibMath::Matrix4 proj = light->GetProjMatrix();
		std::vector<LibMath::Matrix4> views = light->GetViewMatrix(nullptr);

		assert(views.size() == 6 && "incorrect amount of view matrices for a cubemap");

		for (int i = 0; i < 6; i++)
		{
			shadow.m_viewProj[i] = proj * views[i];
		}		

		return true;
	}

	ShadowCubeMap shadow;
	shadow.m_shadowTexture = m_rhi->CreateShadowCubeMap(SHADOW_WIDTH, SHADOW_HEIGHT);

	LibMath::Matrix4 proj = light->GetProjMatrix();
	std::vector<LibMath::Matrix4> views = light->GetViewMatrix(nullptr);

	assert(views.size() == 6 && "incorrect amount of view matrices for a cubemap");

	for (int i = 0; i < 6; i++)
	{
		shadow.m_viewProj[i] = proj * views[i];
	}

	m_shadowCubeMaps.push_back(shadow);
	light->SetShadowIdx(m_shadowCubeMaps.size() - 1);

	return true;
}
