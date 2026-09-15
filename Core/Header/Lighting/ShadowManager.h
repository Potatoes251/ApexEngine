#ifndef SHADOW_MANAGER
#define SHADOW_MANAGER

#include "RHI.h"

#include "ResourceHandle.h"

#include "LibMath/Matrix/Matrix4.h"

#include <array>

class Shader;

namespace Apex::Rendering 
{
	class Scene;
	class Camera;
}

namespace Apex::Lighting
{
	class LightComponent;
	class DirectionalLightComponent;
	class PointLightComponent;
	class SpotLightComponent;
	struct VisibleLights;

	static constexpr unsigned CASCADE_COUNT = 4;
	
	struct CascadedShadowMap
	{
		LibMath::Matrix4       m_viewProj[CASCADE_COUNT];
		Rendering::RHITextureHandle m_shadowTextures[CASCADE_COUNT];
	};

	struct ShadowMap
	{
		LibMath::Matrix4 m_viewProj;
		Rendering::RHITextureHandle m_shadowTexture = Rendering::RHI_INVALID;
	};

	struct ShadowCubeMap
	{
		LibMath::Matrix4 m_viewProj[6];
		Rendering::RHITextureHandle m_shadowTexture = Rendering::RHI_INVALID;
	};

	class ShadowManager
	{
	public:
		ShadowManager(Rendering::IRHI* rhi, Resources::ResourceHandle<Shader> dirShader, Resources::ResourceHandle<Shader> omniShader);
		~ShadowManager();

		void RenderShadows(VisibleLights const& lights, Rendering::Scene* scene, Rendering::Camera* camera);
		void BindShadowMaps(Resources::ResourceHandle<Shader> shader);

	private:
		bool AcquireCascadedShadowMap(DirectionalLightComponent* light, Rendering::Camera* camera, int idx);
		bool AcquireShadowMap(SpotLightComponent* light, Rendering::Camera* camera, int idx);
		bool AcquireShadowCubeMap(PointLightComponent* light, int idx);

		LibMath::Matrix4 ComputeCascadeLightMatrix(Rendering::Camera* camera, DirectionalLightComponent* light, float nearSplit, float farSplit);
		static std::array<float, CASCADE_COUNT> ComputeSplitDepths(float near, float far, float lambda = 0.75f);

		static constexpr unsigned SHADOW_WIDTH = 2048;
		static constexpr unsigned SHADOW_HEIGHT = 2048;
		static constexpr unsigned MAX_SHADOW_2D = 10;
		static constexpr unsigned MAX_SHADOW_CUBE = 5;
		static constexpr unsigned FIRST_SHADOW_2D_SLOT = 5;
		static constexpr unsigned FIRST_SHADOW_CUBE_SLOT = FIRST_SHADOW_2D_SLOT + MAX_SHADOW_2D;

		// depth of each cascade
		std::array<float, CASCADE_COUNT> m_splitDepths;

		std::vector<CascadedShadowMap>	m_cascadedShadowMaps;
		std::vector<ShadowMap>			m_shadowMaps;
		std::vector<ShadowCubeMap>		m_shadowCubeMaps;

		Resources::ResourceHandle<Shader>	m_dirShader;
		Resources::ResourceHandle<Shader>	m_omniShader;

		Rendering::IRHI*					m_rhi;
		Rendering::RHIFrameBufferHandle		m_shadowFbo;

		Rendering::RHIUniformBufferHandle	m_lightMatrixBuffer;
	};
}

#endif // !SHADOW_MANAGER