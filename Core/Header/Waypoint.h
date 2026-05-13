#ifndef WAYPOINT
#define WAYPOINT

#include "Component.h"
#include "Object.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Pathfinding
{
	class WaypointComponent : public Component
	{
	public:
		WaypointComponent() = default;

		LibMath::Vector3 const& GetPosition() const{ return GetOwner()->GetGlobalPosition(); }

		void Serialize(std::ostream& out) const {}
		std::vector<ExposedVar> GetExposedVariables() { return{}; }
		std::unique_ptr<Component> Clone() { return std::make_unique<WaypointComponent>(*this); }

		char const* GetTypeName() const override { return "Waypoint"; }

		void SetGraphId(size_t id) { m_graphId = id; }
		float GetRadius() const { return m_connectRadius; }
	private:
		size_t m_graphId = SIZE_MAX;
		float m_connectRadius = 5.f;
	};
}

#endif // !WAYPOINT