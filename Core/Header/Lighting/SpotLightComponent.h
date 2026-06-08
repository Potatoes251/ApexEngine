#ifndef SPOT_LIGHT_COMPONENT
#define SPOT_LIGHT_COMPONENT


#include "LightComponent.h"

namespace Apex::Lighting
{
	class SpotLightComponent : public LightComponent
	{
	public:
		SpotLightComponent() = default;
		SpotLightComponent(LibMath::Vector3 color, float intensity, float innerCutoff, float outerCutoff);
		~SpotLightComponent() = default;

		std::unique_ptr<Component> Clone() override { return std::make_unique<SpotLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "SpotLight"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		void FillGpuData(void* data_out) override;

		LibMath::Matrix4	GetProjMatrix() const override;
		std::vector<LibMath::Matrix4>	GetViewMatrix(Rendering::Camera*) const override;

		float GetInnerCutoff() const { return m_innerCutoff; }
		float GetOuterCutoff() const { return m_outerCutoff; }

	private:
		float m_innerCutoff = 0.64f;
		float m_outerCutoff = 0.25f;
		float m_nearShadow = .1f;
		float m_farShadow = 10.f;
	};
}


#endif // !SPOT_LIGHT_COMPONENT