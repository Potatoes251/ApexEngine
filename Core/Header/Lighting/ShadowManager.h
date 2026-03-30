#ifndef SHADOW_MANAGER
#define SHADOW_MANAGER

#include "VisibleLights.h"
#include "RHI.h"

#include "ResourceHandle.h"
#include "Shader.h"

#include "LibMath/Matrix/Matrix4.h"

namespace Apex::Rendering 
{
	class Scene;
	class Camera;
}

namespace Apex::Lighting
{
	class LightComponent;

	struct ShadowMap
	{
		LibMath::Matrix4 m_viewProj;
		Rendering::RHITextureHandle m_shadowTexture = Rendering::RHI_INVALID;
		int m_ownerLightId = -1;
	};

	struct ShadowCubeMap
	{
		LibMath::Matrix4 m_viewProj[6];
		Rendering::RHITextureHandle m_shadowTexture = Rendering::RHI_INVALID;
		int m_ownerLightId = -1;
	};

	class ShadowManager
	{
	public:
		ShadowManager(Rendering::IRHI* rhi, Resources::ResourceHandle<Shader> dirShader, Resources::ResourceHandle<Shader> omniShader);
		~ShadowManager();

		void RenderShadows(VisibleLights const& lights, Rendering::Scene* scene, Rendering::Camera* camera);
		void BindShadowMaps(Resources::ResourceHandle<Shader> shader) const;

		void ResetShadowMapId(int shadowIdx) { m_shadowMaps[shadowIdx].m_ownerLightId = -1; }

	private:
		// return the idx of the first available shadowmap 
		// may create a new one if no map is available
		// return -1 if the max amount of shadow map has been reached
		int AcquireShadowMap(LightComponent* light, Rendering::Camera* camera);
		int AcquireShadowCubeMap(LightComponent* light);

		static constexpr unsigned SHADOW_WIDTH = 1024;
		static constexpr unsigned SHADOW_HEIGHT = 1024;
		static constexpr unsigned MAX_SHADOW_2D = 5;
		static constexpr unsigned MAX_SHADOW_CUBE = 5;
		static constexpr unsigned FIRST_SHADOW_2D_SLOT = 5;
		static constexpr unsigned FIRST_SHADOW_CUBE_SLOT = FIRST_SHADOW_2D_SLOT + MAX_SHADOW_2D;


		std::vector<ShadowMap>		m_shadowMaps;
		std::vector<ShadowCubeMap>	m_shadowCubeMaps;

		Resources::ResourceHandle<Shader>	m_dirShader;
		Resources::ResourceHandle<Shader>	m_omniShader;
		Rendering::IRHI*					m_rhi;
		Rendering::RHIFrameBufferHandle		m_shadowFbo;
	};
}

#endif // !SHADOW_MANAGER