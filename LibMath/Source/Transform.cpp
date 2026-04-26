#include "LibMath/Transform.h"

#include "LibMath/Vector/Vector4.h"

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

LibMath::Vector3 LibMath::Transform::getForward() const
{
	return m_rotation.rotate(LibMath::Vector3::front());
}

LibMath::Transform LibMath::Transform::operator*(Transform const& other) const
{
	Transform result;
	result.m_position = m_position + m_rotation.rotate(m_scale * other.m_position);
	result.m_rotation = m_rotation * other.m_rotation;
	result.m_scale = m_scale * other.m_scale;
	return result;
}

LibMath::Transform::operator LibMath::Matrix4() const
{
	return LibMath::Matrix4::createTranslation(m_position) * static_cast<LibMath::Matrix4>(m_rotation) * LibMath::Matrix4::createScale(m_scale);
}

LibMath::Vector3 LibMath::Transform::getEulerRotation() const
{
	return m_rotation.toEuler();
}

void LibMath::Transform::setEulerRotation(Vector3 euler)
{
	m_rotation = Quaternion(
		Radian(euler[0]),
		Radian(euler[1]),
		Radian(euler[2])
	);
}

LibMath::Transform LibMath::Transform::inversed() const
{
	Transform result;

	result.m_scale = { 1.f / m_scale[0], 1.f / m_scale[1], 1.f / m_scale[2]};

	result.m_rotation = m_rotation.conjugated();

	result.m_position = -(result.m_rotation.rotate(result.m_scale * m_position));

	return result;
}