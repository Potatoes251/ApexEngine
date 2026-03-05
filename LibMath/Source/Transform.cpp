#include "LibMath/Transform.h"

LibMath::Transform::Transform(Transform const& other)
{
	m_rotation = other.m_rotation;
	m_position = other.m_position;
	m_scale = other.m_scale;
}

LibMath::Transform& LibMath::Transform::operator=(Transform const other)
{
	m_rotation = other.m_rotation;
	m_position = other.m_position;
	m_scale = other.m_scale;
	return *this;
}

LibMath::Transform::operator LibMath::Matrix4() const
{
	return LibMath::Matrix4::createTranslation(m_position) * static_cast<LibMath::Matrix4>(m_rotation) * LibMath::Matrix4::createScale(m_scale);
}
