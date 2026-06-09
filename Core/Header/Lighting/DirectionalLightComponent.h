#ifndef DIRECTIONAL_LIGHT_COMPONENT
#define DIRECTIONAL_LIGHT_COMPONENT

#include "LightComponent.h"

namespace Apex::Lighting
{
	class DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent() = default;
		DirectionalLightComponent(LibMath::Vector3 color, float intensity, float shadowDistance, float shadowExtent) 
			: LightComponent(color, intensity), m_shadowDistance(shadowDistance), m_shadowExtents(shadowExtent) {}
		~DirectionalLightComponent() = default;

		std::unique_ptr<Component> Clone() override { return std::make_unique<DirectionalLightComponent>(*this); }

		void Serialize(std::ostream& out) const override;
		const char* GetTypeName() const override { return "DirectionalLight"; }

		void FillGpuData(void* data_out) override;

		LibMath::Matrix4	GetProjMatrix() const override;
		std::vector<LibMath::Matrix4>	GetViewMatrix(Rendering::Camera* camera) const override;
		LibMath::Matrix4 GetViewMatrix(std::array<LibMath::Vector3, 8> const& sliceCorners) const;	// CSM version
	private:
		float m_shadowDistance = 25.f;
		float m_shadowExtents = 10.f;
	};
}

#endif // !DIRECTIONAL_LIGHT_COMPONENT

