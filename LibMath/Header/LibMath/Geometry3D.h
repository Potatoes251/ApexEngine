#ifndef GEOMETRY3D_H
#define GEOMETRY3D_H

#include "Angle.h"
#include "Vector/Vector3.h"	
#include "Matrix/Matrix3.h"	
#include "Arithmetic.h"

namespace LibMath
{
	class Point3D
	{
	public:
		Point3D();
		Point3D(float, float, float);					// parametrized constructor
		Point3D(Point3D const&);						// copy constructor
		Point3D(Point3D const&&) noexcept;				// move constructor


		Point3D& operator=(Point3D const&);				// copy asignement
		Point3D& operator=(Point3D const&&) noexcept;	// move asignement

		Point3D operator-() noexcept;

		Vector3 operator-(Point3D const&) const noexcept;
		Vector3 operator+(Point3D const&) const noexcept;
		Vector3 operator-(Vector3 const&) const noexcept;
		Vector3 operator+(Vector3 const&) const noexcept;

		float			getX() const;					// getter for x value
		float			getY() const;					// getter for y value
		float			getZ() const;					// getter for z value

		float			distance(Point3D const&) const;		// return the distance between 2 point
		float			distanceSquare(Point3D const&) const;	// return the square of the distance between 2 point

	private:
		float m_x;
		float m_y;
		float m_z;
	};


	class Line3D
	{
	public:

		Line3D();									// default constructor
		Line3D(Point3D, Vector3);					// parametrized constructor

		Point3D			getCenter() const;			// return the slope of the line
		Vector3			getDirection() const;		// return the slope intercept of the line


	private:
		Point3D m_center;
		Vector3 m_direction;
	};

	class Plane
	{
	public:
		Plane();
		Plane(Vector3, float);

		Vector3		getNormal();			// getter for the normal
		float		getDistance() const;	// getter for the distance from origin
		Point3D		getPointOnPlane();		// get a point on the plane

	private:
		Vector3		m_normal;				// Plane normal (unit vector)
		float		m_distance;             // Distance from the origin
	};


	class Prism
	{
	public:

		Prism();										// default constructor
		Prism(Point3D, float, float, float);			// parameterized construtor without rotation
		Prism(Point3D, float, float, float, Matrix3x3);	// parameterized construtor with rotation

		float			extentX() const;				// return the half-width
		float			extentY() const;				// return the half-height
		float			extentZ() const;				// return the half-height

		float			getWidth() const;				// getter for the width
		float			getHeight() const;				// getter for the height
		float			getDepth() const;				// getter for the depth

		Matrix3x3		getRotation() const;			// getter for the rotation
		Point3D			getCenter() const;				// getter for the center

	private:
		Point3D		m_center;
		float		m_width;
		float		m_height;
		float		m_depth;
		Matrix3x3   m_rotation;
	};


	class Sphere
	{
	public:

		Sphere();								// default constructor
		Sphere(Point3D, float);					// parametrized constructor
		Sphere(Sphere const&);					// copy constructor

		float			getRadius() const;		// getter for the radius
		Point3D			getCenter() const;		// getter for the center

	private:
		Point3D m_center;
		float m_radius;
	};

	class Capsule
	{
	public:

		Capsule();								// default constructor
		Capsule(Point3D, Point3D, float);		// parametrized constructor
		Capsule(Capsule const&);				// copy constructor

		float			getRadius() const;		// getter for the radius
		Point3D			getStart() const;		// getter for the center
		Point3D			getEnd() const;			// getter for the center

	private:
		Point3D m_start;
		Point3D m_end;
		float m_radius;
	};

}


#endif