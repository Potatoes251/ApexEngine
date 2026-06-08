#include "Lighting/ShadowManager.h"

#include "Lighting/VisibleLights.h"
#include "Lighting/Lights.h"

#include "Scene.h"

#include "Shader.h"
#include "Camera.h"

#include <cassert>

using namespace Apex::Lighting;
using namespace Apex::Rendering;

ShadowManager::ShadowManager(IRHI* rhi, Resources::ResourceHandle<Shader> dirShader, Resources::ResourceHandle<Shader> omniShader) 
	: m_rhi(rhi), m_dirShader(dirShader), m_omniShader(omniShader)
{
	assert(rhi != nullptr && "Shadows cannot work without the rhi");

	ShadowMap shadow;
	shadow.m_shadowTexture = rhi->CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

	m_shadowFbo = rhi->CreateEmptyFrameBuffer(shadow.m_shadowTexture, SHADOW_WIDTH, SHADOW_HEIGHT);

	m_shadowMaps.push_back(shadow);

	m_lightMatrixBuffer = rhi->CreateUniformBuffer(3, sizeof(LibMath::Matrix4) * MAX_SHADOW_2D);
}

ShadowManager::~ShadowManager()
{
	if (m_rhi)
	{
		m_rhi->DeleteFrameBuffer(m_shadowFbo);

		for (CascadedShadowMap& shadow : m_cascadedShadowMaps)
		{
			for (int i = 0; i < CASCADE_COUNT; i++)
				m_rhi->DeleteTexture(shadow.m_shadowTextures[i]);
		}

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
	m_rhi->SetCullingFace(true);

	RenderPassInfo pass;
	pass.m_flags |= ShadowPass;
	pass.m_flags |= IgnoreMaterial;
	pass.m_flags &= ~RenderSkybox;
	pass.m_shader = m_dirShader;
	m_dirShader->Use();

	int shadowCount = 0;
	for (DirectionalLightComponent* light : lights.m_directionalLights)
	{
		if (!light->IsCastingShadow())
		{
			light->SetShadowIdx(-1);
			continue;
		}

		if (!AcquireCascadedShadowMap(light, camera, shadowCount)) continue;

		CascadedShadowMap& shadow = m_cascadedShadowMaps[shadowCount];

		m_splitDepths = ComputeSplitDepths(camera->GetNear(), camera->GetFar(), 1);

		for (int cascade = 0; cascade < CASCADE_COUNT; cascade++)
		{
			float nearSplit = (cascade == 0) ? camera->GetNear() : m_splitDepths[cascade - 1];
			float farSplit = m_splitDepths[cascade];

			shadow.m_viewProj[cascade] = ComputeCascadeLightMatrix(camera, light, nearSplit, farSplit);

			m_rhi->AttachDepthTexture(shadow.m_shadowTextures[cascade]);
			m_rhi->ClearDepth();

			m_dirShader->SetUniform("lightSpaceMatrix", shadow.m_viewProj[cascade]);
			scene->Render(pass);
		}

		shadowCount++;
	}
	shadowCount = 0;
	for (SpotLightComponent* light : lights.m_spotLights)
	{
		if (!light->IsCastingShadow()) 
		{
			light->SetShadowIdx(-1);
			continue;
		}

		if (!AcquireShadowMap(light, camera, shadowCount)) continue;

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
		if (!light->IsCastingShadow())
		{
			light->SetShadowIdx(-1);
			continue;
		}

		if (!AcquireShadowCubeMap(light, shadowCount)) continue;

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

void ShadowManager::BindShadowMaps(Resources::ResourceHandle<Shader> shader)
{
	shader->Use();
	shader->SetUniform("cascadeSplits", m_splitDepths[0], CASCADE_COUNT);

	LibMath::Matrix4 lightSpaceMatrix[MAX_SHADOW_2D];
	for (int i = 0; i < m_cascadedShadowMaps.size(); i++)
	{
		CascadedShadowMap& csm = m_cascadedShadowMaps[i];

		for (int c = 0; c < CASCADE_COUNT; c++)
		{
			int textureUnit = FIRST_SHADOW_2D_SLOT + (i * CASCADE_COUNT) + c;
			m_rhi->BindTexture(csm.m_shadowTextures[c], textureUnit);
			lightSpaceMatrix[(i * CASCADE_COUNT) + c] = csm.m_viewProj[c];
		}
	}

	int nbCSM = CASCADE_COUNT * m_cascadedShadowMaps.size();
	for (int i = 0; i < m_shadowMaps.size(); i++)
	{
		ShadowMap& shadow = m_shadowMaps[i];

		int textureUnit = FIRST_SHADOW_2D_SLOT + i + nbCSM;
		m_rhi->BindTexture(shadow.m_shadowTexture, textureUnit);

		lightSpaceMatrix[i + nbCSM] = shadow.m_viewProj;
	}

	m_rhi->BindUniformBuffer(m_lightMatrixBuffer, sizeof(LibMath::Matrix4) * MAX_SHADOW_2D, &lightSpaceMatrix);

	for (int i = 0; i < m_shadowCubeMaps.size(); i++)
	{
		ShadowCubeMap& shadow = m_shadowCubeMaps[i];

		int textureUnit = FIRST_SHADOW_CUBE_SLOT + i;
		m_rhi->BindTextureCube(shadow.m_shadowTexture, textureUnit);
	}
}

bool ShadowManager::AcquireCascadedShadowMap(DirectionalLightComponent* light, Rendering::Camera* camera, int idx)
{
	if (idx >= MAX_SHADOW_2D / CASCADE_COUNT) return false;

	assert(idx >= 0 && idx <= m_cascadedShadowMaps.size());

	if (idx < m_cascadedShadowMaps.size())
	{
		light->SetShadowIdx(idx);
		return true;
	}

	CascadedShadowMap csm;
	for (int c = 0; c < CASCADE_COUNT; c++)
		csm.m_shadowTextures[c] = m_rhi->CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

	m_cascadedShadowMaps.push_back(csm);
	light->SetShadowIdx(m_cascadedShadowMaps.size() - 1);
	return true;
}

bool ShadowManager::AcquireShadowMap(SpotLightComponent* light, Camera* camera, int idx)
{
	if (idx >= MAX_SHADOW_2D) return false;

	assert(idx >= 0 && idx <= m_shadowMaps.size());

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

bool ShadowManager::AcquireShadowCubeMap(PointLightComponent* light, int idx)
{
	if (idx >= MAX_SHADOW_CUBE) return false;

	assert(idx >= 0 && idx <= m_shadowCubeMaps.size());

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

LibMath::Matrix4 ShadowManager::ComputeCascadeLightMatrix(Rendering::Camera* camera, DirectionalLightComponent* light, float nearSplit, float farSplit)
{
	std::array<LibMath::Vector3, 8> corners = camera->GetFrustrumCorners();

	float camNear = camera->GetNear();
	float camFar = camera->GetFar();

	float invDenom = 1 / (camFar - camNear);

	// get the corner of a slice of the frustrum
	for (int i = 0; i < 4; i++)
	{
		LibMath::Vector3 near = corners[i];
		LibMath::Vector3 far = corners[i + 4];
		LibMath::Vector3 ray = far - near;

		float t0 = (nearSplit - camNear) * invDenom;
		float t1 = (farSplit - camNear) * invDenom;

		corners[i] = near + ray * t0;		// near split plane
		corners[i + 4] = near + ray * t1;	// far split plane
	}

	LibMath::Matrix4 lightView = light->GetViewMatrix(corners);

	LibMath::Vector3 minBounds(FLT_MAX, FLT_MAX, FLT_MAX);
	LibMath::Vector3 maxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (LibMath::Vector3& corner : corners)
	{
		LibMath::Vector4 lightSpacePt = lightView * LibMath::Vector4(corner, 1.f);

		minBounds[0] = std::min(minBounds[0], lightSpacePt[0]);
		maxBounds[0] = std::max(maxBounds[0], lightSpacePt[0]);
		minBounds[1] = std::min(minBounds[1], lightSpacePt[1]);
		maxBounds[1] = std::max(maxBounds[1], lightSpacePt[1]);
		minBounds[2] = std::min(minBounds[2], lightSpacePt[2]);
		maxBounds[2] = std::max(maxBounds[2], lightSpacePt[2]);
	}

	float zRange = maxBounds[2] - minBounds[2];
	minBounds[2] -= zRange * 5.0f;
	maxBounds[2] += zRange * 5.0f;

	// Snap to texel boundaries to avoid shimmering when moving around
	float texelSizeX = (maxBounds[0] - minBounds[0]) / SHADOW_WIDTH;
	float texelSizeY = (maxBounds[1] - minBounds[1]) / SHADOW_HEIGHT;

	minBounds[0] = LibMath::floor(minBounds[0] / texelSizeX) * texelSizeX;
	minBounds[1] = LibMath::floor(minBounds[1] / texelSizeY) * texelSizeY;
	maxBounds[0] = LibMath::ceiling(maxBounds[0] / texelSizeX) * texelSizeX;
	maxBounds[1] = LibMath::ceiling(maxBounds[1] / texelSizeY) * texelSizeY;

	LibMath::Matrix4 lightProj = LibMath::Matrix4::orthogonal(minBounds[0], maxBounds[0], minBounds[1], maxBounds[1], minBounds[2], maxBounds[2]);

	return lightProj * lightView;
}

std::array<float, CASCADE_COUNT> ShadowManager::ComputeSplitDepths(float near, float far, float lambda)
{
	std::array<float, CASCADE_COUNT> splits;
	float range = far - near;
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		float p = (i + 1) / (float)CASCADE_COUNT;
		float log = near * std::pow(far / near, p);
		float uni = near + range * p;
		splits[i] = lambda * log + (1.f - lambda) * uni;
	}
	return splits;
}