#ifndef WAYPOINT_GRAPH
#define WAYPOINT_GRAPH

#include <vector>
#include <unordered_map>
#include <string>

#include "LibMath/Vector/Vector3.h"

namespace Apex::Physic { class PhysicSystem; }

namespace Apex::Pathfinding
{
	struct Waypoint
	{
		LibMath::Vector3 m_position;
		int32_t m_id;
		std::unordered_map<int32_t, float> m_neighbors;	// maps neighbors to distance
	};

	class NavGenVolume;

	class WaypointGraph
	{
	public:
		WaypointGraph() = default;

		void GenerateGraph(std::string const& path, Physic::PhysicSystem* physic, NavGenVolume* param);

		void Load(std::string const& path);

		int GetClosestWaypoint(LibMath::Vector3 const& pos);

		std::vector<LibMath::Vector3> GetPath(int startId, int endId);

		void DebugRender();

	private:
		void AddWaypoint(LibMath::Vector3 const& pos);
		void BuildNeighbors(float spacing);

		void Save() const;

		std::vector<Waypoint> m_waypoints;
		std::string m_path;
		int32_t m_nextId = 0;
	};
}

#endif // !WAYPOINT_GRAPH

