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

		std::unique_ptr<Component> Clone() override { return std::make_unique<PointLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "PointLight"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		void FillGpuData(void* data_out) override;

		float GetRadius() const { return m_radius; }
		float GetFarPlane() const { return m_farShadow; }

		LibMath::Matrix4	GetProjMatrix() const override;
		std::vector<LibMath::Matrix4>	GetViewMatrix(Rendering::Camera*) const override;
	private:
		float m_radius = 10.f;
		float m_nearShadow = .1f;
		float m_farShadow = 10.f;
	};
}


#endif // !POINT_LIGHT_COMPONENT

