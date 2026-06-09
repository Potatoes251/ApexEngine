#include "ViewComponent.h"

#include "LibMath/Angle.h"
#include "Collider.h"
#include "Object.h"
#include "Application.h"

using namespace Apex::Perception;
using namespace Apex::Physic;
using namespace Apex;

ViewComponent::ViewComponent(PhysicSystem* physic, float range, float horizontalFov, float verticalFov, int horizontalRays, int verticalRays) :
	m_physicSystem(physic), m_range(range),
	m_horizontalFov(horizontalFov), m_verticalFov(verticalFov),
	m_horizontalRays(horizontalRays), m_verticalRays(verticalRays) {}


ViewComponent::ViewComponent(const ViewComponent& other) : 
	m_physicSystem(other.m_physicSystem), m_range(other.m_range), 
	m_horizontalFov(other.m_horizontalFov), m_verticalFov(other.m_verticalFov),
	m_horizontalRays(other.m_horizontalRays), m_verticalRays(other.m_verticalRays) ,
	m_localDirections(other.m_localDirections) {}

void ViewComponent::Serialize(std::ostream& out) const
{
	out << "        \"range\": "			<< m_range				<< ",\n";
	out << "		\"horizontalFov\": "	<< m_horizontalFov		<< ",\n";
	out << "		\"verticalFov\": "		<< m_verticalFov		<< ",\n";
	out << "		\"horizontalRays\": "	<< m_horizontalRays		<< ",\n";
	out << "		\"verticalRays\": "		<< m_verticalRays		<< "\n";
}

std::unique_ptr<Component> ViewComponent::Clone()
{
	return std::make_unique<ViewComponent>(*this);
}

std::vector<ExposedVar> ViewComponent::GetExposedVariables()
{
	return
	{
		{ "Range",					ExposedVar::Float,	&m_range },
		{ "Horizontal Fov (RAD)",	ExposedVar::Float,	&m_horizontalFov },
		{ "Vertical Fov (RAD)",		ExposedVar::Float,	&m_verticalFov },
		{ "Horizontal Rays",		ExposedVar::Int,	&m_horizontalRays },
		{ "Vertical Rays",			ExposedVar::Int,	&m_verticalRays },
	};
}

void ViewComponent::OnFixedUpdate(float fixedDeltaTime_s)
{
	m_hits.clear();

	LibMath::Vector3 origin = GetOwner()->GetGlobalPosition();
	LibMath::Quaternion rotation = GetOwner()->GetGlobalRotation();

	//Raycasts
	for (const LibMath::Vector3& localDir : m_localDirections)
	{
		LibMath::Vector3 worldDir = rotation.rotate(localDir);
		CastRay(origin, worldDir);
	}
}

void ViewComponent::OnStart()
{
	m_physicSystem = Application::Get()->GetScene()->GetPhysic();

	m_localDirections.reserve((size_t)m_horizontalRays * (size_t)m_verticalRays);

	float halfH = m_horizontalFov * 0.5f;
	float halfV = m_verticalFov * 0.5f;

	for (int y = 0; y < m_verticalRays; ++y)
	{
		float ty = (m_verticalRays > 1) ? (float)y / (m_verticalRays - 1) : 0.5f;
		float pitch = -halfV + ty * m_verticalFov;

		for (int x = 0; x < m_horizontalRays; ++x)
		{
			float tx = (m_horizontalRays > 1) ? (float)x / (m_horizontalRays - 1) : 0.5f;
			float yaw = -halfH + tx * m_horizontalFov;

			LibMath::Vector3 dir;
			dir[0] = std::tan(yaw);
			dir[1] = std::tan(pitch);
			dir[2] = 1.0f;

			m_localDirections.push_back(dir.normalized());
		}
	}
}

bool ViewComponent::SeesTag(std::string const& tag) const
{
	for (HitResult const& hit : m_hits)
	{
		Object* obj = Application::Get()->GetScene()->GetObjectWithId(hit.m_objId);
		if (obj && obj->HasTag(tag))
			return true;
	}
	return false;
}

Apex::Data::Object* ViewComponent::GetClosestWithTag(std::string const& tag) const
{
	float minDist = FLT_MAX;
	Object* closest = nullptr;
	for (HitResult const& hit : m_hits)
	{
		Object* obj = Application::Get()->GetScene()->GetObjectWithId(hit.m_objId);
		bool hasTag = obj && obj->HasTag(tag);
		if (hasTag && hit.m_distance <= minDist)
		{
			minDist = hit.m_distance;
			closest = obj;
		}
	}
	return closest;
}

float ViewComponent::GetShortestDistanceWithTag(std::string const& tag) const
{
	float minDist = FLT_MAX;
	for (HitResult const& hit : m_hits)
	{
		Object* obj = Application::Get()->GetScene()->GetObjectWithId(hit.m_objId);
		bool hasTag = obj && obj->HasTag(tag);
		if (hasTag && hit.m_distance < minDist)
		{
			minDist = hit.m_distance;
		}
	}
	return minDist == FLT_MAX ? -1.f : minDist;
}

void ViewComponent::CastRay(const LibMath::Vector3& origin, const LibMath::Vector3& dir)
{
	std::vector<HitResult> hits;

	if (m_physicSystem->Raycast(hits, origin, dir, m_range))
	{
		for (HitResult& hit : hits)
		{
			Object* obj = Application::Get()->GetScene()->GetObjectWithId(hit.m_objId);
			if (obj && obj != GetOwner())
				m_hits.push_back(hit);
		}
	}
}
