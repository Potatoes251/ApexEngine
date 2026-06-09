#include "Camera.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

using namespace Apex::Rendering;

Camera::Camera(LibMath::Vector3 position)
{
	m_position = position;
}

void Camera::SetProjectionMatrix(float aspect, float near, float far)
{
	m_near = near;
	m_far = far;

	m_proj = LibMath::Matrix4::perspective(LibMath::Degree(m_fov), aspect, near, far);
	m_viewProj = m_proj * m_view;
}

void Camera::SetYaw(float degree)
{
	m_yaw = LibMath::Degree(degree);
	m_dirty = true;
}

void Camera::SetPitch(float degree)
{
	m_pitch = LibMath::Degree(LibMath::clamp(degree, -89.f, 89.f));
	m_dirty = true;
}

LibMath::Matrix4 Camera::GetViewProj()
{
	if (m_dirty) UpdateMatrices();
	return m_viewProj;
}

LibMath::Matrix4 Camera::GetProjection()
{
	return m_proj;
}

LibMath::Matrix4 Camera::GetViewMatrix()
{
	if (m_dirty) UpdateMatrices();
	return m_view;
} 

std::array<LibMath::Vector3, 8> Apex::Rendering::Camera::GetFrustrumCorners()
{ 
	std::array<LibMath::Vector3, 8> frustrumCorners;

	LibMath::Matrix4 inverseViewProj = GetViewProj();
	inverseViewProj.inverse();

	int i = 0;
	for (unsigned int z = 0; z < 2; ++z)
	{
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				const LibMath::Vector4 pt =
					inverseViewProj * LibMath::Vector4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				frustrumCorners[i++] = pt / pt[3];
			}
		}
	}

	return frustrumCorners;
}

void Camera::SetPosition(LibMath::Vector3 pos)
{ 
	m_position = pos; 
	m_dirty = true;
}

void Camera::OnStart()
{
	SetProjectionMatrix();
}

std::vector<Apex::ExposedVar> Camera::GetExposedVariables()
{
	return { { "Main Camera" , Apex::ExposedVar::Bool, &m_isMain } };
}
