#include "Camera.h"

#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"
#include "LibMath/Trigonometry.h"

namespace Apex::Rendering
{
	Camera::Camera(LibMath::Vector3 position)
	{
		m_position = position;
	}

	void Camera::SetProjectionMatrix(float aspect, float near, float far)
	{
		m_proj = LibMath::Matrix4::perspective(LibMath::Degree(m_fov), aspect, near, far);
		m_viewProj = m_proj * m_view;
	}
	
	LibMath::Matrix4 Camera::GetViewProj()
	{
		if (m_dirty) UpdateMatrices();
		return m_viewProj;
	}
	
	LibMath::Matrix4 Camera::GetProjection()
	{
		if (m_dirty) UpdateMatrices();
		return m_proj;
	}
	
	LibMath::Matrix4 Camera::GetViewMatrix()
	{
		if (m_dirty) UpdateMatrices();
		return m_view;
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
}