#ifndef SPOT_LIGHT_COMPONENT
#define SPOT_LIGHT_COMPONENT


#include "LightComponent.h"

namespace Apex::Lighting
{
	class SpotLightComponent : public LightComponent
	{
	public:
		SpotLightComponent() = default;
		~SpotLightComponent() = default;
		SpotLightComponent(SpotLightComponent const&) {}
		SpotLightComponent& operator=(SpotLightComponent const&) {}

		std::unique_ptr<Component> Clone() const override { return std::make_unique<SpotLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "SpotLightComponent"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		void FillGpuData(void* data_out) override;

		float GetInnerCutoff() const { return m_innerCutoff; }
		float GetOuterCutoff() const { return m_outerCutoff; }
	private:
		float m_innerCutoff = 50.f;
		float m_outerCutoff = 75.f;
	};
}


#endif // !SPOT_LIGHT_COMPONENT