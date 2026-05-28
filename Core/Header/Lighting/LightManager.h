#ifndef LIGHT_MANAGER
#define LIGHT_MANAGER

#include "VisibleLights.h"
#include "RHI.h"

#include "GpuLightStructs.h"

namespace Apex::Lighting
{
	class LightManager
	{
	public:
		LightManager(Rendering::IRHI* rhi);
		~LightManager();

		void UploadToGpu(VisibleLights const& lights);

	private:
		Rendering::IRHI* m_rhi;

		Rendering::RHIUniformBufferHandle m_pointLightBuffer;
		Rendering::RHIUniformBufferHandle m_spotLightBuffer;
		Rendering::RHIUniformBufferHandle m_directionalLightBuffer;
	};
}

#endif // !LIGHT_MANAGER

