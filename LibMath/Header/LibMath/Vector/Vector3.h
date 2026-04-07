#ifndef LIBMATH_VECTOR_VECTOR3_H_
#define LIBMATH_VECTOR_VECTOR3_H_

#include <iostream>
#include <string>

namespace LibMath
{
	class Quaternion;
	class Vector2;
	class Vector4;
	class Radian;

	class Vector3
	{
	public:
						Vector3();										// set all component to 0
		explicit		Vector3(float);									// set all component to the same value
						Vector3(float, float, float);					// set all component individually
						Vector3(Vector2 const&, float);
						Vector3(Vector3 const&);						// copy all component
						Vector3(Vector4 const&);						// copy all component
						~Vector3() = default;

		static Vector3	zero();											// return a vector with all its component set to 0
		static Vector3	one();											// return a vector with all its component set to 1
		static Vector3	up();											// return a unit vector pointing upward
		static Vector3	down();											// return a unit vector pointing downward
		static Vector3	left();											// return a unit vector pointing left
		static Vector3	right();										// return a unit vector pointing right
		static Vector3	front();										// return a unit vector pointing forward
		static Vector3	back();											// return a unit vector pointing backward

		Vector3&		operator=(Vector3 const&);

		float&			operator[](int);								// return this vector component value
		float			operator[](int) const;							// return this vector component value

		Radian			angleFrom(Vector3 const&) const;				// return smallest angle between 2 vector

		Vector3			cross(Vector3 const&) const;					// return a copy of the cross product result

		float			distanceFrom(Vector3 const&) const;				// return distance between 2 points
		float			distanceSquaredFrom(Vector3 const&) const;		// return square value of the distance between 2 points
		float			distance2DFrom(Vector3 const&) const;			// return the distance between 2 points on the X-Y axis only
		float			distance2DSquaredFrom(Vector3 const&) const;	// return the square value of the distance between 2 points points on the X-Y axis only

		float			dot(Vector3 const&) const;						// return dot product result

		bool			isLongerThan(Vector3 const&) const;				// return true if this vector magnitude is greater than the other
		bool			isShorterThan(Vector3 const&) const;			// return true if this vector magnitude is less than the other

		bool			isUnitVector() const;							// return true if this vector magnitude is 1

		float			magnitude() const;								// return vector magnitude
		float			magnitudeSquared() const;						// return square value of the vector magnitude

		Vector3&		normalize();									// scale this vector to have a magnitude of 1
		Vector3			normalized() const;								// scale this vector to have a magnitude of 1

		void			projectOnto(Vector3 const&);					// project this vector onto an other

		void			reflectOnto(Vector3 const&);					// reflect this vector by an other

		void			rotate(Radian, Radian, Radian);					// rotate this vector using Euler angle apply in the z, x, y order
		void			rotate(Radian, Vector3 const&);					// rotate this vector around an arbitrary axis
		void			rotate(Quaternion const&);						// rotate this vector using a quaternion rotor

		void			scale(Vector3 const&);							// scale this vector by a given factor

		std::string		string() const;									// return a string representation of this vector
		std::string		stringLong() const;								// return a verbose string representation of this vector

		void			translate(Vector3 const&);						// offset this vector by a given distance

		static Vector3		lerp(Vector3 const&, Vector3 const&, float);

	private:
		float m_x;
		float m_y;
		float m_z;
	};

	bool			operator==(Vector3 const&, Vector3 const&);			// Vector3{ 1 } == Vector3::one()				// true					// return if 2 vectors have the same component
	bool			operator!=(Vector3 const&, Vector3 const&);			// Vector3{ 1 } != Vector3::one()				// false				// return if 2 vectors differ by at least a component

	Vector3			operator-(Vector3);									// - Vector3{ .5, 1.5, -2.5 }					// { -.5, -1.5, 2.5 }	// return a copy of a vector with all its component inverted

	Vector3			operator*(Vector3 const&, float);
	Vector3			operator/(Vector3 const&, float);
	Vector3&		operator*=(Vector3&, float);
	Vector3&		operator/=(Vector3&, float);

	Vector3			operator+(Vector3, Vector3 const&);					// Vector3{ .5, 1.5, -2.5 } + Vector3::one()	// { 1.5, 2.5, -1.5 }	// add 2 vectors component wise
	Vector3			operator-(Vector3, Vector3 const&);					// Vector3{ .5, 1.5, -2.5 } - Vector3{ 1 }		// { -.5, .5, -3.5 }	// subtract 2 vectors component wise
	Vector3			operator*(Vector3, Vector3 const&);					// Vector3{ .5, 1.5, -2.5 } * Vector3::zero()	// { 0, 0, 0 }			// multiply 2 vectors component wise
	Vector3			operator/(Vector3, Vector3 const&);					// Vector3{ .5, 1.5, -2.5 } / Vector3{ 2 }		// { .25, .75, -1.25 }	// divide 2 vectors component wise

	Vector3&		operator+=(Vector3&, Vector3 const&);				// addition component wise
	Vector3&		operator-=(Vector3&, Vector3 const&);				// subtraction component wise
	Vector3&		operator*=(Vector3&, Vector3 const&);				// multiplication component wise
	Vector3&		operator/=(Vector3&, Vector3 const&);				// division component wise

	std::ostream&	operator<<(std::ostream&, Vector3 const&);			// cout << Vector3{ .5, 1.5, -2.5 }				// add a vector string representation to an output stream
	//std::istream&	operator>>(std::istream&, Vector3&);				// ifstream file{ save.txt }; file >> vector;	// parse a string representation from an input stream into a vector
}

#endif // !LIBMATH_VECTOR_VECTOR3_H_
