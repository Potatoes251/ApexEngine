#include "FirstPersonCamera.h"

#include "../../Resources/Header/Object.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

using namespace Apex::Data;
using namespace Apex::Rendering;

FirstPersonCamera::FirstPersonCamera(LibMath::Vector3 position)
{
	m_position = position;
	UpdateVectors();
}

FirstPersonCamera::FirstPersonCamera(FirstPersonCamera const& other)
{
	m_isMain = other.m_isMain;
	m_speed = other.m_speed;
	m_fov = other.m_fov;
	m_yaw = other.m_yaw;
	m_pitch = other.m_pitch;
	m_position = other.m_position;
	m_isMain = other.m_isMain;
}

FirstPersonCamera& FirstPersonCamera::operator=(FirstPersonCamera const& other)
{
	m_isMain = other.m_isMain;
	m_speed = other.m_speed;
	m_fov = other.m_fov;
	m_yaw = other.m_yaw;
	m_pitch = other.m_pitch;
	m_position = other.m_position;
	m_isMain = other.m_isMain;

	return *this;
}

void FirstPersonCamera::ProcessKeyboard(bool moveForward, bool moveBackward, bool moveLeft,
	bool moveRight, bool moveUp, bool moveDown, float deltaTime)
{
	float velocity = m_speed * deltaTime;

	if (moveForward) m_position += m_front * velocity;
	if (moveBackward) m_position -= m_front * velocity;
	if (moveLeft) m_position -= m_right * velocity;
	if (moveRight) m_position += m_right * velocity;
	if (moveUp) m_position += LibMath::Vector3::up() * velocity;
	if (moveDown) m_position -= LibMath::Vector3::up() *velocity;

	m_dirty = true;
}

void FirstPersonCamera::UpdateVectors()
{
	m_front[0] = LibMath::cos(m_yaw) * LibMath::cos(m_pitch);
	m_front[1] = LibMath::sin(m_pitch);
	m_front[2] = LibMath::sin(m_yaw) * LibMath::cos(m_pitch);
	m_front.normalize();

	m_right = m_front.cross(LibMath::Vector3::up()).normalize();
	m_up = m_right.cross(m_front).normalize();

	m_dirty = true;
}

void FirstPersonCamera::UpdateMatrices()
{
	m_view = LibMath::Matrix4::lookAt(m_position, m_position + m_front, m_up);
	m_viewProj = m_proj * m_view;
	m_dirty = false;
}

void FirstPersonCamera::ProcessMouse(double dx, double dy, float sensitivity)
{
	if (!IsEnabled()) return;

	m_yaw += LibMath::Degree((float)dx * sensitivity);
	m_pitch += LibMath::Degree((float)dy * sensitivity);
	m_pitch = LibMath::Degree(LibMath::clamp(m_pitch.raw(), -89.f, 89.f));
	UpdateVectors();
}

void FirstPersonCamera::ProcessScroll(double yoffset)
{
	if (!IsEnabled()) return;

	m_speed *= (yoffset > 0) ? 1.1f : (1.f / 1.1f);
	m_speed = LibMath::clamp(m_speed, 0.5f, m_maxSpeed);
}

void FirstPersonCamera::Serialize(std::ostream& out) const
{
	out << "        \"isMain\": " << (m_isMain ? "true" : "false") << "\n";
}

void FirstPersonCamera::OnLateUpdate(float /*deltatime_s*/)
{
	Object* owner = GetOwner();
	if (!owner) return;

	SetPosition(owner->GetGlobalTransform().getPosition());
}

void FirstPersonCamera::SetOrientation(LibMath::Degree pitch, LibMath::Degree yaw)
{
	m_pitch = LibMath::Degree(LibMath::clamp(pitch.raw(), -89.f, 89.f));
	m_yaw = yaw;
	UpdateVectors();
}
