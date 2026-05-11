#include "ViewComponent.h"

#include "LibMath/Angle.h"
#include "Rigidbody.h"
#include "Object.h"

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
	out << "      \"ViewComponent\": {\n";
	out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";

	out << "        \"range\": "			<< m_range				<< ",\n";
	out << "		\"horizontalFov\": "	<< m_horizontalFov		<< ",\n";
	out << "		\"verticalFov\": "		<< m_verticalFov		<< ",\n";
	out << "		\"horizontalRays\": "	<< m_horizontalRays		<< ",\n";
	out << "		\"verticalRays\": "		<< m_verticalRays		<< "\n";
	out << "      }";
}

std::unique_ptr<Component> ViewComponent::Clone()
{
	auto clone = std::make_unique<ViewComponent>(*this);
	return clone;
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
	m_Hits.clear();

	Vector3 origin = GetOwner()->GetGlobalPosition();
	Quaternion rotation = GetOwner()->GetGlobalRotation();

	//Raycasts
	for (const Vector3& localDir : m_localDirections)
	{
		Vector3 worldDir = rotation.rotate(localDir);
		CastRay(origin, worldDir);
	}
}

void ViewComponent::OnStart()
{
	m_localDirections.reserve(m_horizontalRays * m_verticalRays);

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

			Vector3 dir;
			dir[0] = std::tan(yaw);
			dir[1] = std::tan(pitch);
			dir[2] = 1.0f;

			m_localDirections.push_back(dir.normalized());
		}
	}
}

void ViewComponent::CastRay(const Vector3& origin, const Vector3& dir)
{
	HitResult hit;

	if (m_physicSystem->Raycast(hit, origin, dir, m_range) &&
		hit.m_body->GetOwner()->GetVisibility())
	{
		m_Hits.push_back(hit);
	}
}
