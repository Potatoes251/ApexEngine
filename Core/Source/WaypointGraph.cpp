#include "WaypointGraph.h"

#include "Waypoint.h"

#include <queue>
#include <unordered_set>
#include <unordered_map>

using namespace Apex::Pathfinding;

void WaypointGraph::BuildGraph(std::vector<WaypointComponent*> waypoints)
{
	for (size_t i = 0; i < waypoints.size(); i++)
	{
		WaypointComponent* wp = waypoints[i];
		wp->SetGraphId(i);
		m_waypoints.push_back(Waypoint{ wp->GetPosition(), (int)i });
	}

	for (size_t i = 0; i < waypoints.size(); i++)
	{
		for (size_t j = i + 1; j < waypoints.size(); j++)
		{
			float dist = m_waypoints[i].m_position.distanceFrom(m_waypoints[j].m_position);

			float radius = std::min(waypoints[i]->GetRadius(), waypoints[j]->GetRadius());

			if (dist <= radius)
			{
				m_waypoints[i].m_neighbors[j] = dist;
				m_waypoints[j].m_neighbors[i] = dist;
			}
		}
	}
}

int WaypointGraph::GetClosestWaypoint(LibMath::Vector3 pos)
{
	int closest = -1;
	float minSquaredDist = FLT_MAX;
	for (Waypoint const& wp : m_waypoints)
	{
		float squaredDist = pos.distanceSquaredFrom(wp.m_position);
		if (minSquaredDist > squaredDist)
		{
			minSquaredDist = squaredDist;
			closest = wp.m_id;
		}
	}

	return closest;
}

std::vector<LibMath::Vector3> WaypointGraph::GetPath(int startId, int endId)
{
	int size = (int)(m_waypoints.size());
	if (m_waypoints.empty() || startId >= size || endId >= size || startId < 0 || endId < 0)
		return{};

	if (startId == endId)
		return { m_waypoints[startId].m_position };

	Waypoint end = m_waypoints[endId];

	// (f) score -> id
	using Node = std::pair<float, int>;
	// to be eveluated
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet{};
	openSet.push({ 0, startId });
	// already evaluated
	std::unordered_set<int> closedSet{};

	// cost to reach node n from the start
	std::unordered_map<int, float> globalScore; //(g)
	globalScore[startId] = 0;
	// a came from b
	std::unordered_map<int, int> cameFrom;

	while (!openSet.empty())
	{
		auto [cost, current] = openSet.top();
		openSet.pop();

		if (closedSet.contains(current)) continue;
		if (current == endId) break;

		closedSet.emplace(current);

		for (auto& [neighbor, dist] : m_waypoints[current].m_neighbors)
		{
			if (closedSet.contains(neighbor)) continue;

			// estimated distance to the target (h)
			float estimate = m_waypoints[neighbor].m_position.distanceFrom(end.m_position);
			float newGlobal = globalScore[current] + dist;

			float total = newGlobal + estimate;

			if (!globalScore.contains(neighbor) || newGlobal < globalScore[neighbor])
			{
				openSet.push({ total, neighbor });

				globalScore[neighbor] = newGlobal;
				cameFrom[neighbor] = current;
			}
		}
	}

	std::vector<LibMath::Vector3> path{};

	// build the list of position from then end
	int currentId = endId;
	while (currentId != startId)
	{
		if (!cameFrom.contains(currentId)) return {};

		path.push_back(m_waypoints[currentId].m_position);

		currentId = cameFrom[currentId];
	}
	// add the start point
	path.push_back(m_waypoints[currentId].m_position);

	std::reverse(path.begin(), path.end());

	return path;
}
