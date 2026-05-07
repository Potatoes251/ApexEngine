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
		WaterVolume(LibMath::Vector3 halfExtents, float density, float drag, float buoyancyMultiplier, LibMath::Vector3 flowDir, float flowSpeed);

		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() override { return std::make_unique<WaterVolume>(*this); }

		const char* GetTypeName() const override { return "WaterVolume"; }
		std::vector<ExposedVar> GetExposedVariables() override;

		bool IsInside(LibMath::Vector3 const& pos) const;

		LibMath::Vector3 const& GetHalfExtents() const { return m_halfExtents; }

		float GetDensity() const { return m_density; }
		float GetDrag() const { return m_drag; }
		float GetBuoyancyMult() const { return m_buoyancyMultiplier; }
		LibMath::Vector3 GetFlowDir() const { return m_flowDirection; }
		float GetFlowSpeed() const { return m_flowSpeed; }

	private:
		LibMath::Vector3 m_halfExtents;

		// Physics
		float m_density = 1000;                // e.g. 1000 for water
		float m_drag = 0;                   // single drag value (0-1, higher = more resistance)
		float m_buoyancyMultiplier = 1;     // 1.0 = realistic, 0.5 = half buoyancy, 2.0 = bouncy

		// Movement
		LibMath::Vector3 m_flowDirection{};        // current direction (normalized)
		float m_flowSpeed = 0;              // m/s of current at full strength
	};
}

#endif // !WATER_VOLUME