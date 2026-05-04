#ifndef WATER_VOLUME
#define WATER_VOLUME

#include "Component.h"

#include "LibMath/Vector/Vector3.h"

namespace Apex::Physic
{
	class WaterVolume : public Component
	{
	public:
		WaterVolume() = default;
		WaterVolume(float density, float drag, float buoyancyMultiplier, LibMath::Vector3 flowDir, float flowSpeed)
			: m_density(density), m_drag(drag), m_buoyancyMultiplier(buoyancyMultiplier), 
			m_flowDirection(flowDir), m_flowSpeed(flowSpeed) {}

		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() override { return std::make_unique<WaterVolume>(*this); }

		const char* GetTypeName() const override { return "WaterVolume"; }
		std::vector<ExposedVar> GetExposedVariables() override;

	private:
		LibMath::Vector3 m_halfExtents;

		// Physics
		float m_density;                // e.g. 1000 for water
		float m_drag;                   // single drag value (0-1, higher = more resistance)
		float m_buoyancyMultiplier;     // 1.0 = realistic, 0.5 = half buoyancy, 2.0 = bouncy

		// Movement
		LibMath::Vector3 m_flowDirection;        // current direction (normalized)
		float m_flowSpeed;              // m/s of current at full strength
	};
}

#endif // !WATER_VOLUME