#ifndef DIRECTIONAL_LIGHT_COMPONENT
#define DIRECTIONAL_LIGHT_COMPONENT

#include "LightComponent.h"

namespace Apex::Lighting
{
	class DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent() = default;
		DirectionalLightComponent(LibMath::Vector3 color, float intensity) : LightComponent(color, intensity) {}
		~DirectionalLightComponent() = default;
		DirectionalLightComponent(DirectionalLightComponent const&);
		DirectionalLightComponent& operator=(DirectionalLightComponent const& other);

		std::unique_ptr<Component> Clone() const override { return std::make_unique<DirectionalLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "DirectionalLight"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		void FillGpuData(void* data_out) override;

		LibMath::Matrix4	GetProjMatrix() const override;
		std::vector<LibMath::Matrix4>	GetViewMatrix(Rendering::Camera* camera) const override;
	private:
		float m_shadowDistance = 25.f;
	};
}

#endif // !DIRECTIONAL_LIGHT_COMPONENT

