#ifndef GEOMETRY2D_H
#define GEOMETRY2D_H

#include "Angle.h"

namespace LibMath
{
	class Point2D
	{
	public:

		Point2D();										// default constructor
		Point2D(float, float);							// parametrized constructor
		Point2D(Point2D const&);						// copy constructor
		Point2D(Point2D const&&) noexcept;				// move constructor

		Point2D& operator=(Point2D const&);				// copy asignement
		Point2D& operator=(Point2D const&&) noexcept;	// move asignement

		float			getX() const;					// getter for x value
		float			getY() const;					// getter for y value

		float			distance(Point2D const&) const;		// return the distance between 2 point
		float			distanceSquare(Point2D const&) const;	// return the square of the distance between 2 point

	private:
		float m_x;
		float m_y;
	};


	class Line2D
	{
	public:

		Line2D();									// default constructor
		Line2D(Point2D, Point2D);					// parametrized constructor

		Point2D			getStart() const;			// return the start point
		Point2D			getEnd() const;				// return the end point

		float			length() const;				// return the length of the line
		float			lengthSquare() const;		// return the square of the length of the line
		float			slope() const;				// return the slope of the line
		float			slopeIntercept() const;		// return the slope intercept of the line


	private:
		Point2D m_start;
		Point2D m_end;
	};


	class Rectangle
	{
	public:

		Rectangle();									// default constructor
		Rectangle(Point2D, float, float);				// parameterized construtor without rotation
		Rectangle(Point2D, float, float, Radian);		// parameterized construtor with rotation

		float			extentX() const;				// return the half-width
		float			extentY() const;				// return the half-height
		Radian			getRotation() const;			// return the rotation
		float			getWidth() const;				// getter for the width
		float			getHeight() const;				// getter for the height
		Point2D			getCenter() const;				// return the center

		void			getCorners(Point2D corners[4]);	// return the 4 corners

	private:
		Point2D	m_center;
		float m_width;
		float m_height;
		Radian m_rotation;
	};


	class Circle
	{
	public:

		Circle();								// default constructor
		Circle(Point2D, float);					// parametrized constructor
		Circle(Circle const&);					// copy constructor

		float			getRadius() const;		// getter for the radius
		Point2D			getCenter() const;		// getter for the center

	private:
		Point2D m_center;
		float m_radius;
	};
}

#endif