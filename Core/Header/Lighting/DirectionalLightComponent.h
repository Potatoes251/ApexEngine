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
	private:

	};
}

#endif // !DIRECTIONAL_LIGHT_COMPONENT

