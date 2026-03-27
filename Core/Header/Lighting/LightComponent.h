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
		LightComponent(LibMath::Vector3 color, float intensity);
		virtual ~LightComponent() = default;
		LightComponent(LightComponent const&) = delete;
		LightComponent& operator=(LightComponent const&) = delete;

		virtual std::vector<ExposedVar> GetExposedVariables() override;

		virtual void FillGpuData(void* data_out) = 0;

		LibMath::Vector3	GetColor() const { return m_color; }
		float				GetIntensity() const { return m_intensity; }
	protected:
		LibMath::Vector3 m_color = LibMath::Vector3(1.f, 1.f, 1.f);
		float m_intensity = 10.f;
	};
}



#endif // !LIGHT_COMPONENT

