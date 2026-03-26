#ifndef LIGHT_COMPONENT
#define LIGHT_COMPONENT

#include "Component.h"

#include "LibMath/Vector/Vector3.h"

namespace Apex::Lighting
{
	class LightComponent : public Component
	{
	public:
		LightComponent() = default;
		virtual ~LightComponent() = default;
		LightComponent(LightComponent const&) = delete;
		LightComponent& operator=(LightComponent const&) = delete;

		virtual std::vector<ExposedVar> GetExposedVariables() override;

	protected:
		LibMath::Vector3 m_color;
		float m_intensity = 10.f;
	};
}



#endif // !LIGHT_COMPONENT

