#ifndef WAYPOINT
#define WAYPOINT

#include "Component.h"
#include "Object.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Pathfinding
{
	class NavGenVolume : public Component
	{
	public:
		NavGenVolume() = default;

		void Serialize(std::ostream& out) const;
		std::vector<ExposedVar> GetExposedVariables();
		std::unique_ptr<Component> Clone() { return std::make_unique<NavGenVolume>(*this); }

		char const* GetTypeName() const override { return "NavGenVolume"; }

		LibMath::Vector3 GetMinBound();
		LibMath::Vector3 GetMaxBound();

		float   m_spacing = 1.f;
		float   m_genHeight = 1.f;
	};
}

#endif // !WAYPOINT