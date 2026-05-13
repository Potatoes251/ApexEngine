#ifndef WAYPOINT_GRAPH
#define WAYPOINT_GRAPH

#include <vector>
#include <unordered_map>

#include "LibMath/Vector/Vector3.h"

namespace Apex::Pathfinding
{
	struct Waypoint
	{
		LibMath::Vector3 m_position;
		int m_id;
		std::unordered_map<int, float> m_neighbors;	// maps neighbors to distance
	};
	
	class WaypointComponent;

	class WaypointGraph
	{
	public:
		WaypointGraph() = default;

		void BuildGraph(std::vector<WaypointComponent*> waypoints);
		int GetClosestWaypoint(LibMath::Vector3 pos);

		std::vector<LibMath::Vector3> GetPath(int startId, int endId);

	private:
		std::vector<Waypoint> m_waypoints;
	};
}

#endif // !WAYPOINT_GRAPH

