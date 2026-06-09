#include "ThirdPersonCamera.h"

#include "../../Resources/Header/Object.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

using namespace Apex::Data;
using namespace Apex::Rendering;

ThirdPersonCamera::ThirdPersonCamera(LibMath::Vector3 target, float distance)
{
	m_target = target;
	m_distance = distance;
}

ThirdPersonCamera::ThirdPersonCamera(ThirdPersonCamera const& other)
{
	m_isMain = other.m_isMain;
	m_speed = other.m_speed;
	m_target = other.m_target;
	m_distance = other.m_distance;
	m_fov = other.m_fov;
	m_yaw = other.m_yaw;
	m_pitch = other.m_pitch;
	m_position = other.m_position;
	m_isMain = other.m_isMain;
}

ThirdPersonCamera& Apex::Rendering::ThirdPersonCamera::operator=(ThirdPersonCamera const& other)
{
	m_isMain = other.m_isMain;
	m_speed = other.m_speed;
	m_target = other.m_target;
	m_distance = other.m_distance;
	m_fov = other.m_fov;
	m_yaw = other.m_yaw;
	m_pitch = other.m_pitch;
	m_position = other.m_position;
	m_isMain = other.m_isMain;

	return *this;
}

void ThirdPersonCamera::ProcessMouse(double dx, double dy, float sensitivity)
{
	if (!IsEnabled()) return;

	m_yaw += LibMath::Degree((float)dx * sensitivity);
	m_pitch += LibMath::Degree((float)dy * sensitivity);
	m_pitch = LibMath::Degree(LibMath::clamp(m_pitch.raw(), -89.f, 89.f));

	m_dirty = true;
}

void ThirdPersonCamera::Serialize(std::ostream& out) const
{
	out << "        \"isMain\": " << (m_isMain ? "true" : "false") << ",\n";
	out << "        \"distance\": " << m_distance << ",\n";
	out << "        \"speed\": " << m_speed << "\n";
}

std::vector<Apex::ExposedVar> ThirdPersonCamera::GetExposedVariables()
{
	std::vector<Apex::ExposedVar> var = Camera::GetExposedVariables();

	var.push_back({ "Distance to Target" , Apex::ExposedVar::Float, &m_distance });
	var.push_back({ "Follow Speed" , Apex::ExposedVar::Float, &m_speed });

	return var;
}

void ThirdPersonCamera::Teleport(LibMath::Vector3 pos)
{
	Camera::Teleport(pos);
	m_target = pos;
}

void ThirdPersonCamera::OnStart()
{
	Camera::OnStart();
	Teleport(GetOwner()->GetGlobalPosition());
}

void ThirdPersonCamera::OnLateUpdate(float deltatime_s)
{
	Object* owner = GetOwner();
	if (owner)
	{
		float lerpFactor = 1.0f - std::exp(-m_speed * deltatime_s);

		m_target = LibMath::Vector3::lerp(m_target, owner->GetGlobalTransform().getPosition(), lerpFactor);
		m_dirty = true;
	}
}

void ThirdPersonCamera::UpdateMatrices()
{
	LibMath::Vector3 direction;
	direction[0] = LibMath::cos(m_yaw) * LibMath::cos(m_pitch);
	direction[1] = LibMath::sin(m_pitch);
	direction[2] = LibMath::sin(m_yaw) * LibMath::cos(m_pitch);
	direction.normalize();
	m_position = m_target - direction * m_distance;
	m_view = LibMath::Matrix4::lookAt(m_position, m_target, LibMath::Vector3::up());
	m_viewProj = m_proj * m_view;
	m_dirty = false;
}
