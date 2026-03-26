#ifndef POINT_LIGHT_COMPONENT
#define POINT_LIGHT_COMPONENT

#include "LightComponent.h"

namespace Apex::Lighting
{
	class PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent() = default;
		~PointLightComponent() = default;
		PointLightComponent(PointLightComponent const&) {}
		PointLightComponent& operator=(PointLightComponent const&) {}

		std::unique_ptr<Component> Clone() const override { return std::make_unique<PointLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "PointLightComponent"; }
		std::vector<ExposedVar> GetExposedVariables() override;
		void DrawVariables(Apex::UserInterface::IGUI& gui) override {}
	private:
		float m_radius = 10.f;
	};
}


#endif // !POINT_LIGHT_COMPONENT

