#define _USE_MATH_DEFINES
#include <cmath>

#include "LibMath/Geometry3D.h"

LibMath::Point3D::Point3D() : 
	m_x(0), 
	m_y(0), 
	m_z(0) 
{}

LibMath::Point3D::Point3D(float x, float y, float z) : 
	m_x(x), 
	m_y(y), 
	m_z(z) 
{}

LibMath::Point3D::Point3D(Point3D const& other) : 
	m_x(other.m_x), 
	m_y(other.m_y), 
	m_z(other.m_z) 
{}

LibMath::Point3D::Point3D(Point3D const&& other) noexcept : 
	m_x(other.m_x), 
	m_y(other.m_y), 
	m_z(other.m_z) 
{}

LibMath::Point3D&		LibMath::Point3D::operator=(Point3D const& other)
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_z = other.m_z;

	return *this;
}

LibMath::Point3D&		LibMath::Point3D::operator=(Point3D const&& other) noexcept
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_z = other.m_z;

	return *this;
}

LibMath::Point3D	LibMath::Point3D::operator-() noexcept
{
	return Point3D(-m_x, -m_y, -m_z);
}

LibMath::Vector3	LibMath::Point3D::operator-(Point3D const& other) const noexcept
{
	return Vector3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
}

LibMath::Vector3	LibMath::Point3D::operator+(Point3D const& other) const noexcept
{
	return Vector3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
}

LibMath::Vector3	LibMath::Point3D::operator-(Vector3 const& other) const noexcept
{
	return Vector3(m_x - other[0], m_y - other[1], m_z - other[2]);
}

LibMath::Vector3	LibMath::Point3D::operator+(Vector3 const& other) const noexcept
{
	return Vector3(m_x + other[0], m_y + other[1], m_z + other[2]);
}

float		LibMath::Point3D::getX() const
{
	return m_x;
}

float		LibMath::Point3D::getY() const
{
	return m_y;
}

float		LibMath::Point3D::getZ() const
{
	return m_z;
}

float		LibMath::Point3D::distance(Point3D const& other) const
{
	return LibMath::squareRoot((m_x - other.m_x) * (m_x - other.m_x) +
								(m_y - other.m_y) * (m_y - other.m_y) +
								(m_z - other.m_z) * (m_z - other.m_z));
}

float		LibMath::Point3D::distanceSquare(Point3D const& other) const
{
	return ((m_x - other.m_x) * (m_x - other.m_x) + 
			(m_y - other.m_y) * (m_y - other.m_y) + 
			(m_z - other.m_z) * (m_z - other.m_z));
}




LibMath::Line3D::Line3D() :
	m_center(),
	m_direction()
{}

LibMath::Line3D::Line3D(Point3D center, Vector3 direction) :
	m_center(center),
	m_direction(direction)
{
	m_direction.normalize();
}

LibMath::Point3D	LibMath::Line3D::getCenter() const
{
	return m_center;
}

LibMath::Vector3	LibMath::Line3D::getDirection() const
{
	return m_direction;
}




LibMath::Sphere::Sphere() :
	m_center(),
	m_radius(0)
{}

LibMath::Sphere::Sphere(Point3D center, float radius) :
	m_center(center),
	m_radius(radius)
{}

LibMath::Sphere::Sphere(Sphere const& other) :
	m_center(other.m_center),
	m_radius(other.m_radius)
{}

float				LibMath::Sphere::getRadius() const
{
	return m_radius;
}

LibMath::Point3D LibMath::Sphere::getCenter() const
{
	return m_center;
}




LibMath::Capsule::Capsule() :
	m_start(),
	m_end(),
	m_radius(0)
{}

LibMath::Capsule::Capsule(Point3D start, Point3D end, float radius) :
	m_start(start),
	m_end(end),
	m_radius(radius)
{}

LibMath::Capsule::Capsule(Capsule const& other) :
	m_start(other.m_start),
	m_end(other.m_end),
	m_radius(other.m_radius)
{}

float				LibMath::Capsule::getRadius() const
{
	return m_radius;
}

LibMath::Point3D	LibMath::Capsule::getStart() const
{
	return m_start;
}

LibMath::Point3D	LibMath::Capsule::getEnd() const
{
	return m_end;
}




LibMath::Plane::Plane() :
	m_normal(),
	m_distance(0)
{}

LibMath::Plane::Plane(Vector3 normal, float distance) :
	m_normal(normal.normalize()),
	m_distance(distance)
{}

LibMath::Vector3 LibMath::Plane::getNormal()
{
	return m_normal;
}

float LibMath::Plane::getDistance() const
{
	return m_distance;
}

LibMath::Point3D LibMath::Plane::getPointOnPlane()
{
	m_normal.normalize();
	Vector3 point = -m_normal * m_distance;
	return Point3D(point[0], point[1], point[2]);
}




LibMath::Prism::Prism() :
	m_center(),
	m_width(0),
	m_height(0),
	m_depth(0),
	m_rotation()
{}

LibMath::Prism::Prism(Point3D center, float width, float height, float depth) :
	m_center(center),
	m_width(width),
	m_height(height),
	m_depth(depth),
	m_rotation()
{}

LibMath::Prism::Prism(Point3D center, float width, float height, float depth, Matrix3x3 rotation) :
	m_center(center),
	m_width(width),
	m_height(height),
	m_depth(depth),
	m_rotation(rotation)
{}

float LibMath::Prism::extentX() const
{
	return m_width / 2.0f;
}

float LibMath::Prism::extentY() const
{
	return m_height / 2.0f;
}

float LibMath::Prism::extentZ() const
{
	return m_depth / 2.0f;
}

LibMath::Matrix3x3 LibMath::Prism::getRotation() const
{
	return m_rotation;
}

float LibMath::Prism::getWidth() const
{
	return m_width;
}

float LibMath::Prism::getHeight() const
{
	return m_height;
}

float LibMath::Prism::getDepth() const
{
	return m_depth;
}

LibMath::Point3D LibMath::Prism::getCenter() const
{
	return m_center;
}
