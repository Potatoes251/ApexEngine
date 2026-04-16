#ifndef LIGHT_COMPONENT
#define LIGHT_COMPONENT

#include "Component.h"

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"

namespace Apex::Rendering
{
	class Camera;
}

namespace Apex::Lighting
{
	class LightComponent : public Component
	{
	public:
		LightComponent() = default;
		LightComponent(LibMath::Vector3 color, float intensity);
		virtual ~LightComponent() = default;
		LightComponent(LightComponent const&) = delete;
		LightComponent& operator=(LightComponent const&) = delete;

		virtual std::vector<ExposedVar> GetExposedVariables() override;

		virtual void FillGpuData(void* data_out) = 0;

		virtual LibMath::Matrix4	GetProjMatrix() const = 0;
		virtual std::vector<LibMath::Matrix4>	GetViewMatrix(Rendering::Camera* camera) const = 0;

		void				SetShadowIdx(int idx) { m_shadowIdx = idx; }
		LibMath::Vector3	GetColor() const { return m_color; }
		float				GetIntensity() const { return m_intensity; }
		bool				IsCastingShadow() const { return m_castShadows; }
	protected:
		LibMath::Vector3 m_color = LibMath::Vector3(1.f, 1.f, 1.f);
		float m_intensity = 10.f;
		int m_shadowIdx = -1;
		bool m_castShadows = true;
	};
}



#endif // !LIGHT_COMPONENT

