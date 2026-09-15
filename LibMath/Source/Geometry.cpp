#define _USE_MATH_DEFINES
#include <cmath>

#include "LibMath/Geometry.h"
#include "LibMath/Trigonometry.h"
#include "LibMath/arithmetic.h"

LibMath::Point2D::Point2D() :
	m_x(0),
	m_y(0)
{}

LibMath::Point2D::Point2D(float x, float y) : 
	m_x(x),
	m_y(y)
{}

LibMath::Point2D::Point2D(Point2D const& other) :
	m_x(other.m_x),
	m_y(other.m_y)
{}

LibMath::Point2D::Point2D(Point2D const&& other) noexcept :
	m_x(other.m_x),
	m_y(other.m_y)
{}

LibMath::Point2D& LibMath::Point2D::operator=(Point2D const& other)
{
	m_x = other.m_x;
	m_y = other.m_y;

	return *this;
}

LibMath::Point2D& LibMath::Point2D::operator=(Point2D const&& other) noexcept
{
	m_x = other.m_x;
	m_y = other.m_y;

	return *this;
}

float LibMath::Point2D::getX() const
{
	return m_x;
}

float LibMath::Point2D::getY() const
{
	return m_y;
}

float LibMath::Point2D::distance(Point2D const& other) const
{
	return LibMath::squareRoot((m_x - other.m_x) * (m_x - other.m_x) + (m_y - other.m_y) * (m_y - other.m_y));
}

float LibMath::Point2D::distanceSquare(Point2D const& other) const
{
	return ((m_x - other.m_x) * (m_x - other.m_x) + (m_y - other.m_y) * (m_y - other.m_y));
}

LibMath::Line2D::Line2D() :
	m_start(), 
	m_end() 
{}

LibMath::Line2D::Line2D(Point2D start, Point2D end) :
	m_start(start), 
	m_end(end) 
{}

LibMath::Point2D LibMath::Line2D::getStart() const
{
	return m_start;
}

LibMath::Point2D LibMath::Line2D::getEnd() const
{
	return m_end;
}

float LibMath::Line2D::length() const
{
	return m_start.distance(m_end);
}

float LibMath::Line2D::lengthSquare() const
{
	return m_start.distanceSquare(m_end);
}

float LibMath::Line2D::slope() const
{
	return (m_start.getY() - m_end.getY()) / (m_start.getX() - m_end.getX());
}

float LibMath::Line2D::slopeIntercept() const
{
	return m_start.getY() - slope() * m_start.getX();
}

LibMath::Rectangle::Rectangle() : 
	m_center(),
	m_width(0.f),
	m_height(0.f), 
	m_rotation(0.f) 
{}

LibMath::Rectangle::Rectangle(Point2D center, float width, float height) : 
	m_center(center),
	m_width(width), 
	m_height(height), 
	m_rotation(0.f) 
{}

LibMath::Rectangle::Rectangle(Point2D center, float width, float height, Radian rotation) :
	m_center(center),
	m_width(width), 
	m_height(height), 
	m_rotation(rotation)
{}

float LibMath::Rectangle::extentX() const
{
	return m_width / 2.0f;
}

float LibMath::Rectangle::extentY() const
{
	return m_height / 2.0f;
}

LibMath::Radian LibMath::Rectangle::getRotation() const
{
	return m_rotation;
}

float LibMath::Rectangle::getWidth() const
{
	return m_width;
}

float LibMath::Rectangle::getHeight() const
{
	return m_height;
}

LibMath::Point2D LibMath::Rectangle::getCenter() const
{
	return m_center;
}

void LibMath::Rectangle::getCorners(Point2D corners[4])
{
	if (m_rotation.raw() == 0)
	{
		corners[0] = Point2D(m_center.getX(), m_center.getY()),
		corners[1] = Point2D(m_center.getX() + m_width, m_center.getY()) ,
		corners[2] = Point2D(m_center.getX() + m_width, m_center.getY() - m_height),
		corners[3] = Point2D(m_center.getX(), m_center.getY() - m_height);
	}

	Point2D center = { m_center.getX(), m_center.getY() };

	float half_width = m_width / 2.0f;
	float half_height = m_height / 2.0f;

	float cosRot = cos(m_rotation);
	float sinRot = sin(m_rotation);

	Point2D offsets[4] = { {-half_width, -half_height}, {half_width, -half_height},
						  {half_width, half_height}, {-half_width, half_height} };

	for (int i = 0; i < 4; i++)
	{
		float x = offsets[i].getX() * cosRot - offsets[i].getY() * sinRot;
		float y = offsets[i].getX() * sinRot + offsets[i].getY() * cosRot;
		corners[i] = Point2D(center.getX() + x, center.getY() + y);
	}
}




LibMath::Circle::Circle() : 
	m_center(),
	m_radius(0.f)
{}

LibMath::Circle::Circle(Point2D center, float radius) :
	m_center(center),
	m_radius(radius)
{}

LibMath::Circle::Circle(Circle const& other) : 
	m_center(other.m_center), 
	m_radius(other.m_radius)
{}

float LibMath::Circle::getRadius() const
{
	return m_radius;
}

LibMath::Point2D LibMath::Circle::getCenter() const
{
	return m_center;
}