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
		LightComponent() { m_lightId = m_nextLightId++; }
		LightComponent(LibMath::Vector3 color, float intensity);
		virtual ~LightComponent() = default;
		LightComponent(LightComponent const&) = delete;
		LightComponent& operator=(LightComponent const&) = delete;

		virtual std::vector<ExposedVar> GetExposedVariables() override;

		virtual void FillGpuData(void* data_out) = 0;

		virtual LibMath::Matrix4	GetProjMatrix() const = 0;
		virtual std::vector<LibMath::Matrix4>	GetViewMatrix(Rendering::Camera* camera) const = 0;

		LibMath::Vector3	GetColor() const { return m_color; }
		float				GetIntensity() const { return m_intensity; }
		int					GetShadowIdx() const { return m_shadowIdx; }
		void				SetShadowIdx(int newIdx) { m_shadowIdx = newIdx; }
		int					GetLightId() const { return m_lightId; }
		bool				IsCastingShadow() const { return m_castShadows; }
	protected:
		static inline int m_nextLightId = 0;

		LibMath::Vector3 m_color = LibMath::Vector3(1.f, 1.f, 1.f);
		float m_intensity = 10.f;
		int m_shadowIdx = -1;
		int m_lightId = -1;
		bool m_castShadows = true;
	};
}



#endif // !LIGHT_COMPONENT

