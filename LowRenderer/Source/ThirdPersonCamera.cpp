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

void ThirdPersonCamera::ProcessMouse(double dx, double dy, float sensitivity)
{
	m_yaw += LibMath::Degree((float)dx * sensitivity);
	m_pitch += LibMath::Degree((float)dy * sensitivity);
	m_pitch = LibMath::Degree(LibMath::clamp(m_pitch.raw(), -89.f, 89.f));

	m_dirty = true;
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
