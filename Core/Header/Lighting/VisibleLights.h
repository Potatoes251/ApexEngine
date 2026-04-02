#ifndef VISIBLE_LIGHTS
#define VISIBLE_LIGHTS

#include <vector>

namespace Apex::Lighting
{
	class DirectionalLightComponent;
	class PointLightComponent;
	class SpotLightComponent;


	// used to store lights when rendering
	struct VisibleLights
	{
		std::vector<DirectionalLightComponent*> m_directionalLights;
		std::vector<PointLightComponent*>		m_pointLights;
		std::vector<SpotLightComponent*>		m_spotLights;
	};
}

#endif // !VISIBLE_LIGHTS

