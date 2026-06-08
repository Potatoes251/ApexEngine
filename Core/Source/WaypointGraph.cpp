#include "WaypointGraph.h"

#include "Waypoint.h"

#include "Physic.h"
#include "DebugRenderer.h"

#include "Log.h"
#include "BinLoader.h"


#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <filesystem>

using namespace Apex::Pathfinding;

void WaypointGraph::GenerateGraph(std::string const& path, Physic::PhysicSystem* physic, NavGenVolume* param)
{
	m_path = path;
	m_waypoints.clear();
	m_nextId = 0;
	LibMath::Vector3 min = param->GetMinBound();
	LibMath::Vector3 max = param->GetMaxBound();

	float rayDistance = param->m_genHeight - min[1];

	for (float x = min[0]; x < max[0]; x += param->m_spacing)
	{
		for (float z = min[2]; z < max[2]; z += param->m_spacing)
		{
			Physic::HitResult hit;
			if (physic->Raycast(hit, { x, param->m_genHeight, z }, LibMath::Vector3::down(), rayDistance, true))
			{
				AddWaypoint(hit.m_position);
			}
		}
	}

	BuildNeighbors(param->m_spacing);

	Save();
}

void WaypointGraph::Load(std::string const& path)
{
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open())
	{
		LOG_ERROR_CAT("Pathfinding", "Failed to open bin file : {}", path);
		return;
	}

	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();

	if (size == 0)
	{
		LOG_ERROR_CAT("Pathfinding", "File is empty : {}", path);
		return;
	}
	
	file.seekg(0, std::ios::beg);

	uint32_t nbWaypoint;
	BinLoader::Read32Bits(file, nbWaypoint);

	for (uint32_t i = 0; i < nbWaypoint; i++)
	{
		Waypoint wp;

		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t id;

		BinLoader::Read32Bits(file, x);
		BinLoader::Read32Bits(file, y);
		BinLoader::Read32Bits(file, z);
		BinLoader::Read32Bits(file, id);

		wp.m_position = { std::bit_cast<float>(x), std::bit_cast<float>(y), std::bit_cast<float>(z) };
		wp.m_id = std::bit_cast<int32_t>(id);

		uint32_t nbNeighbor;
		BinLoader::Read32Bits(file, nbNeighbor);

		for (uint32_t i = 0; i < nbNeighbor; i++)
		{
			uint32_t neighborId;
			uint32_t neighborDist;
			BinLoader::Read32Bits(file, neighborId);
			BinLoader::Read32Bits(file, neighborDist);

			wp.m_neighbors[std::bit_cast<int32_t>(neighborId)] = std::bit_cast<float>(neighborDist);
		}
		m_waypoints.push_back(wp);
	}
}

int WaypointGraph::GetClosestWaypoint(LibMath::Vector3 const& pos)
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

void WaypointGraph::DebugRender()
{
	Rendering::DebugRenderer& dr = Rendering::DebugRenderer::Get();

	for (Waypoint const& wp : m_waypoints)
	{
		for (auto const& [neighbor, dist] : wp.m_neighbors)
		{
			if (neighbor > wp.m_id)
			{
				dr.AddLine(wp.m_position, m_waypoints[neighbor].m_position);
			}
		}
	}
}

void WaypointGraph::AddWaypoint(LibMath::Vector3 const& pos)
{
	m_waypoints.push_back({ pos, m_nextId++ });
}

void WaypointGraph::BuildNeighbors(float spacing)
{
	float radius = spacing * 1.5f;	// 1.5f = slightly above sqrt(2) to allow diagonal neighbors
	for (size_t i = 0; i < m_waypoints.size(); i++)
	{
		for (size_t j = i + 1; j < m_waypoints.size(); j++)
		{
			float dist = m_waypoints[i].m_position.distanceFrom(m_waypoints[j].m_position);

			if (dist <= radius)
			{
				m_waypoints[i].m_neighbors[j] = dist;
				m_waypoints[j].m_neighbors[i] = dist;
			}
		}
	}
}

void WaypointGraph::Save() const
{
	std::ofstream file(m_path, std::ios::binary);

	if (!file.is_open())
	{
		LOG_ERROR_CAT("Pathfinding", "Failed to open bin file : {}", m_path);
		return;
	}

	uint32_t size = (uint32_t)m_waypoints.size();
	BinLoader::Write32Bits(file, size);

	for (Waypoint wp : m_waypoints)
	{		
		BinLoader::Write32Bits(file, std::bit_cast<uint32_t>(wp.m_position[0]));
		BinLoader::Write32Bits(file, std::bit_cast<uint32_t>(wp.m_position[1]));
		BinLoader::Write32Bits(file, std::bit_cast<uint32_t>(wp.m_position[2]));
		BinLoader::Write32Bits(file, std::bit_cast<uint32_t>(wp.m_id));

		size = (uint32_t)wp.m_neighbors.size();
		BinLoader::Write32Bits(file, size);

		for (auto const& [neighborId, dist] : wp.m_neighbors)
		{
			BinLoader::Write32Bits(file, std::bit_cast<uint32_t>(neighborId));
			BinLoader::Write32Bits(file, std::bit_cast<uint32_t>(dist));
		}
	}
}


