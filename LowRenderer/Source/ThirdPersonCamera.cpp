#include "ThirdPersonCamera.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

using namespace Apex::Rendering;

ThirdPersonCamera::ThirdPersonCamera(LibMath::Vector3 target, float distance)
{
	m_target = target;
	m_distance = distance;
}

ThirdPersonCamera::ThirdPersonCamera(ThirdPersonCamera const& other)
{
	m_target = other.m_target;
	m_distance = other.m_distance;
	m_fov = other.m_fov;
	m_yaw = other.m_yaw;
	m_pitch = other.m_pitch;
	m_position = other.m_position;
}

ThirdPersonCamera& Apex::Rendering::ThirdPersonCamera::operator=(ThirdPersonCamera const& other)
{
	m_target = other.m_target;
	m_distance = other.m_distance;
	m_fov = other.m_fov;
	m_yaw = other.m_yaw;
	m_pitch = other.m_pitch;
	m_position = other.m_position;

	return *this;
}

void ThirdPersonCamera::ProcessMouse(double dx, double dy, float sensitivity)
{
	m_yaw += LibMath::Degree((float)dx * sensitivity);
	m_pitch += LibMath::Degree((float)dy * sensitivity);
	m_pitch = LibMath::Degree(LibMath::clamp(m_pitch.raw(), -89.f, 89.f));

	m_dirty = true;
}

std::vector<Apex::ExposedVar> ThirdPersonCamera::GetExposedVariables()
{
	return { { "distance to target" ,Apex::ExposedVar::Float, &m_distance } };
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
	m_dirty = false;
}
