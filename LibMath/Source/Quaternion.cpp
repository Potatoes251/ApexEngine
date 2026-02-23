#include "LibMath/Quaternion.h"
#include "LibMath/Arithmetic.h"
#include <LibMath/Angle.h>
#include <LibMath/Vector/Vector3.h>
#include "LibMath/Trigonometry.h"

#include <cassert>

LibMath::Quaternion::Quaternion()
{
	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_w = 0;
}

LibMath::Quaternion::Quaternion(float x, float y, float z, float w)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

LibMath::Quaternion::Quaternion(Quaternion const& other)
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_z = other.m_z;
	m_w = other.m_w;
}

LibMath::Quaternion::Quaternion(Radian pitch, Radian yaw, Radian roll)
{
	float cosx = cos(pitch / 2);
	float sinx = sin(pitch / 2);
	 
	float cosy = cos(yaw / 2);
	float siny = sin(yaw / 2);
	 
	float cosz = cos(roll / 2);
	float sinz = sin(roll / 2);

	m_w = cosx * cosy * cosz + sinx * siny * sinz;
	m_x = sinx * cosy * cosz - cosx * siny * sinz;
	m_y = cosx * siny * cosz + sinx * cosy * sinz;
	m_z = cosx * cosy * sinz - sinx * siny * cosz;
}

LibMath::Quaternion::Quaternion(Radian angle, Vector3 axis)
{
	if (axis.magnitudeSquared() <= g_epsilon)
	{
		*this = identity();
		return;
	}
	axis.normalize();

	angle *= 0.5f;

	float sin = LibMath::sin(angle);
	float cos = LibMath::cos(angle);

	m_x = axis[0] * sin;
	m_y = axis[1] * sin;
	m_z = axis[2] * sin;
	m_w = cos;
}

float LibMath::Quaternion::magnitude() const
{
	return std::sqrt(magnitudeSquared());
}

float LibMath::Quaternion::magnitudeSquared() const
{
	return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
}

bool LibMath::Quaternion::isUnit() const
{
	return fabs(magnitudeSquared() - 1.f) < g_epsilon;
}

LibMath::Quaternion LibMath::Quaternion::conjugated() const
{
	return Quaternion {-m_x, -m_y, -m_z, m_w};
}

LibMath::Quaternion LibMath::Quaternion::inversed() const
{
	if (isUnit())
		return conjugated();

	float invMagSquared = 1 / magnitudeSquared();

	return conjugated() * invMagSquared;
}

LibMath::Quaternion LibMath::Quaternion::normalized() const
{
	float mag = magnitude();
	
	//if (mag <= g_epsilon)
	//	return identity();

	return *this * (1.f / mag);
}

LibMath::Quaternion& LibMath::Quaternion::normalize()
{
	float mag = magnitude();

	//if (mag <= g_epsilon)
	//{
	//	*this = identity();
	//	return *this;
	//}

	m_x /= mag;
	m_y /= mag;
	m_z /= mag;
	m_w /= mag;

	return *this;
}

LibMath::Quaternion& LibMath::Quaternion::conjugate()
{
	m_x = -m_x;
	m_y = -m_y;
	m_z = -m_z;

	return *this;
}

LibMath::Quaternion& LibMath::Quaternion::inverse()
{
	if (isUnit())
	{
		return conjugate();
	}

	float invMagSquared = 1 / magnitudeSquared();

	m_x = -m_x * invMagSquared;
	m_y = -m_y * invMagSquared;
	m_z = -m_z * invMagSquared;
	m_w = m_w * invMagSquared;

	return *this;
}

LibMath::Vector3 LibMath::Quaternion::rotate(Vector3 const& vec) const
{
	//assert(isUnit());

	Quaternion q = *this;
	Quaternion qv(vec[0], vec[1], vec[2], 0.0f);

	Quaternion result = q * qv * q.conjugate();

	return Vector3(
		result.m_x,
		result.m_y,
		result.m_z
	);
}

float LibMath::Quaternion::angleBetween(Quaternion const& other) const
{
	assert(isUnit());
	assert(other.isUnit());

	float dotProduct =
		m_x * other.m_x +
		m_y * other.m_y +
		m_z * other.m_z +
		m_w * other.m_w;
	
	dotProduct = std::min(std::abs(dotProduct), 1.f);

	return 2.f * std::acos(dotProduct);
}

float& LibMath::Quaternion::operator[](int index)
{
	if (index == 0)
	{
		return m_x;
	}
	else if (index == 1)
	{
		return m_y;
	}
	else if (index == 2)
	{
		return m_z;
	}
	else if (index == 3)
	{
		return m_w;
	}
	else
	{
		throw std::out_of_range("Invalid index: Quaternion only supports indices 0, 1, 2 and 3.");
	}
}

float LibMath::Quaternion::operator[](int index) const
{
	if (index == 0)
	{
		return m_x;
	}
	else if (index == 1)
	{
		return m_y;
	}
	else if (index == 2)
	{
		return m_z;
	}
	else if (index == 3)
	{
		return m_w;
	}
	else
	{
		throw std::out_of_range("Invalid index: Quaternion only supports indices 0, 1, 2 and 3.");
	}
}

bool LibMath::Quaternion::operator==(Quaternion const& other) const
{
	return fabs(m_x - other.m_x) < g_epsilon &&
		fabs(m_y - other.m_y) < g_epsilon &&
		fabs(m_z - other.m_z) < g_epsilon &&
		fabs(m_w - other.m_w) < g_epsilon;
}

bool LibMath::Quaternion::operator!=(Quaternion const& other) const
{
	return fabs(m_x - other.m_x) > g_epsilon ||
		fabs(m_y - other.m_y) > g_epsilon ||
		fabs(m_z - other.m_z) > g_epsilon ||
		fabs(m_w - other.m_w) > g_epsilon;
}

LibMath::Quaternion LibMath::Quaternion::operator+(Quaternion const& other) const
{
	return Quaternion(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
}

LibMath::Quaternion LibMath::Quaternion::operator*(Quaternion const& other) const
{
	return Quaternion(
		m_w * other.m_x + m_x * other.m_w + m_y * other.m_z - m_z * other.m_y,
		m_w * other.m_y - m_x * other.m_z + m_y * other.m_w + m_z * other.m_x,
		m_w * other.m_z + m_x * other.m_y - m_y * other.m_x + m_z * other.m_w,
		m_w * other.m_w - m_x * other.m_x - m_y * other.m_y - m_z * other.m_z);
}

LibMath::Quaternion LibMath::Quaternion::operator*(float val) const
{
	return Quaternion(m_x * val, m_y * val, m_z * val, m_w * val);
}

LibMath::Quaternion::operator LibMath::Matrix4() const
{
	//assert(isUnit);

	float xx = m_x * m_x;
	float yy = m_y * m_y;
	float zz = m_z * m_z;
	float xy = m_x * m_y;
	float xz = m_x * m_z;
	float yz = m_y * m_z;
	float wx = m_w * m_x;
	float wy = m_w * m_y;
	float wz = m_w * m_z;

	return Matrix4(
		1.f - 2.f * (yy + zz),	2.f * (xy - wz),		2.f * (xz + wy),		0.f,
		2.f * (xy + wz),		1.f - 2.f * (xx + zz),	2.f * (yz - wx),		0.f,
		2.f * (xz - wy),		2.f * (yz + wx),		1.f - 2.f * (xx + yy),	0.f,
		0.f,					0.f,					0.f,					1.f);
}

LibMath::Quaternion LibMath::Quaternion::slerp(Quaternion from, Quaternion to, float alpha)
{
	// Clamp interpolation factor
	alpha = clamp(alpha, 0.0f, 1.0f);

	// Compute the cosine of the angle between the two quaternions
	float dot =
		from.m_x * to.m_x +
		from.m_y * to.m_y +
		from.m_z * to.m_z +
		from.m_w * to.m_w;

	// Take shortest path
	if (dot < 0.0f)
	{
		dot = -dot;
		to.m_x = -to.m_x;
		to.m_y = -to.m_y;
		to.m_z = -to.m_z;
		to.m_w = -to.m_w;
	}

	if (dot > 0.9995f)
	{
		Quaternion result(
			from.m_x + alpha * (to.m_x - from.m_x),
			from.m_y + alpha * (to.m_y - from.m_y),
			from.m_z + alpha * (to.m_z - from.m_z),
			from.m_w + alpha * (to.m_w - from.m_w)
		);
		return result.normalize();
	}

	float theta0 = std::acos(dot);
	float theta = theta0 * alpha;

	float sinTheta = std::sin(theta);
	float sinTheta0 = std::sin(theta0);

	float s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
	float s1 = sinTheta / sinTheta0;

	return Quaternion(
		s0 * from.m_x + s1 * to.m_x,
		s0 * from.m_y + s1 * to.m_y,
		s0 * from.m_z + s1 * to.m_z,
		s0 * from.m_w + s1 * to.m_w
	);
}

LibMath::Quaternion	LibMath::Quaternion::identity()
{
	return Quaternion(0, 0, 0, 1);
}