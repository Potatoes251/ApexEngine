#ifndef LIGHT_MANAGER
#define LIGHT_MANAGER

#include "VisibleLights.h"
#include "RHI.h"

#include "Light.h"

namespace Apex::Lighting
{
	class LightManager
	{
	public:
		LightManager(Rendering::IRHI* rhi);
		~LightManager();

		void UploadToGpu(VisibleLights const& lights);

	private:

		static inline constexpr size_t m_maxPointLights = 10;
		static inline constexpr size_t m_maxSpotLights = 10;
		static inline constexpr size_t m_maxDirectionalLights = 10;

		Rendering::IRHI* m_rhi;

		Rendering::RHIUniformBufferHandle m_pointLightBuffer;
		Rendering::RHIUniformBufferHandle m_spotLightBuffer;
		Rendering::RHIUniformBufferHandle m_directionalLightBuffer;
	};
}

#endif // !LIGHT_MANAGER

