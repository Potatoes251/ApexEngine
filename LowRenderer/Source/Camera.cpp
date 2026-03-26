#include "Camera.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

namespace Apex::Rendering
{
	Camera::Camera(LibMath::Vector3 position)
	{
		m_up = { 0, -1, 0 };
		m_up = { 0, 1, 0 };
		m_position = position;
		UpdateVectors();
	}

	LibMath::Matrix4 Camera::GetViewMatrix() const
	{
		return LibMath::Matrix4::lookAt(m_position, m_position + m_front, m_up);
	}

	LibMath::Matrix4 Camera::GetProjectionMatrix(float aspect, float near, float far) const
	{
		LibMath::Radian fovRad = LibMath::Degree(m_fov);

		return LibMath::Matrix4::perspective(fovRad.radian(), aspect, near, far);
	}

	void Camera::ProcessKeyboard(bool moveForward, bool moveBackward, bool moveLeft, 
								 bool moveRight, bool moveUp, bool moveDown, float deltaTime)
	{
		float velocity = m_speed * deltaTime;

		if (moveForward) m_position += m_front * velocity;
		if (moveBackward) m_position -= m_front * velocity;
		if (moveLeft) m_position -= m_right * velocity;
		if (moveRight) m_position += m_right * velocity;
		if (moveUp) m_position += m_up * velocity;
		if (moveDown) m_position -= m_up * velocity;
	}

	void Camera::ProcessMouse(double dx, double dy, float sensitivity)
	{
		m_yaw += (float)dx * sensitivity;
		m_pitch += (float)dy * sensitivity;
		m_pitch = LibMath::clamp(m_pitch, -89.f, 89.f);
		UpdateVectors();
	}

	void Camera::ProcessScroll(double yoffset)
	{
		m_fov -= (float)yoffset;
		m_fov = LibMath::clamp(m_fov, 1.f, 90.f);
	}

	void Camera::UpdateVectors()
	{
		LibMath::Radian yawRad = LibMath::Degree(m_yaw);
		LibMath::Radian pitchRad = LibMath::Degree(m_pitch);

		m_front[0] = LibMath::cos(yawRad) * LibMath::cos(pitchRad);
		m_front[1] = LibMath::sin(pitchRad);
		m_front[2] = LibMath::sin(yawRad) * LibMath::cos(pitchRad);
		m_front.normalize();

		m_right = m_front.cross(LibMath::Vector3::up()).normalize();
		//m_up = m_right.cross(m_front).normalize();
	}
}