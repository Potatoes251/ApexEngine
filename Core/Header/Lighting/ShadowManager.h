#ifndef SHADOW_MANAGER
#define SHADOW_MANAGER

#include "VisibleLights.h"
#include "RHI.h"

namespace Apex::Lighting
{
	struct ShadowMap
	{
		Rendering::RHITextureHandle m_shadowTexture;
		int m_ownerLightId = -1;
		bool m_cubemap = false;
	};

	class ShadowManager
	{
	public:
		ShadowManager(Rendering::IRHI* rhi);
		~ShadowManager();

		void RenderShadows(VisibleLights const& lights);
		void BindShadowMaps() const;

	private:
		// return the idx of the first available shadowmap 
		// may create a new one if no map is available
		// return -1 if the max amount of shadow map has been reached
		int AcquireShadowMap(bool cubemap);

		static inline constexpr unsigned m_shadowWidth = 1048;
		static inline constexpr unsigned m_shadowHeight = 1048;

		std::vector<ShadowMap> m_shadowMaps;
		Rendering::IRHI* m_rhi;

		Rendering::RHIFrameBufferHandle m_shadowFbo;
	};
}

#endif // !SHADOW_MANAGER