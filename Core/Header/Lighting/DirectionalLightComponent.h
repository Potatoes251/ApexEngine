#ifndef DIRECTIONAL_LIGHT_COMPONENT
#define DIRECTIONAL_LIGHT_COMPONENT

#include "LightComponent.h"

namespace Apex::Lighting
{
	class DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent() = default;
		~DirectionalLightComponent() = default;
		DirectionalLightComponent(DirectionalLightComponent const&) {}
		DirectionalLightComponent& operator=(DirectionalLightComponent const&) {}

		std::unique_ptr<Component> Clone() const override { return std::make_unique<DirectionalLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "DirectionalLightComponent"; }
		std::vector<ExposedVar> GetExposedVariables() override;
		void DrawVariables(Apex::UserInterface::IGUI& gui) override {}

	private:

	};
}

#endif // !DIRECTIONAL_LIGHT_COMPONENT

