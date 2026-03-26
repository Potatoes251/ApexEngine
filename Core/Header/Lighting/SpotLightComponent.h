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
		void DrawVariables(Apex::UserInterface::IGUI& gui) override {}
	private:
		float m_innerCutOff = 50.f;
		float m_outerCutOff = 75.f;
	};
}


#endif // !SPOT_LIGHT_COMPONENT