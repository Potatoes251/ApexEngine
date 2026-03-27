#ifndef POINT_LIGHT_COMPONENT
#define POINT_LIGHT_COMPONENT

#include "LightComponent.h"

namespace Apex::Lighting
{
	class PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent() = default;
		PointLightComponent(LibMath::Vector3 color, float intensity, float radius);
		~PointLightComponent() = default;
		PointLightComponent(PointLightComponent const& other);
		PointLightComponent& operator=(PointLightComponent const& other);

		std::unique_ptr<Component> Clone() const override { return std::make_unique<PointLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "PointLight"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		void FillGpuData(void* data_out) override;

		float GetRadius() const { return m_radius; }
	private:
		float m_radius = 10.f;
	};
}


#endif // !POINT_LIGHT_COMPONENT

