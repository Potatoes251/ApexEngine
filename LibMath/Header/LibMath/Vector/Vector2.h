#ifndef LIBMATH_VECTOR_VECTOR2_H_
#define LIBMATH_VECTOR_VECTOR2_H_

#include "LibMath/Angle/Radian.h"

namespace LibMath
{
	class Vector2
	{
	public:
						Vector2();										// set all component to 0
		explicit		Vector2(float);									// set all component to the same value
						Vector2(float, float);							// set all component individually
						Vector2(Vector2 const&);						// copy constructor
						Vector2(Vector2 const&&) noexcept;				// move constructor
						~Vector2() = default;

		Vector2&		operator=(Vector2 const&);						// copy asignement
		Vector2&		operator=(Vector2 const&&) noexcept;			// move asignement

		Vector2			operator-();

		Vector2			operator+(Vector2 const&) const;				// add 2 vectors
		Vector2			operator-(Vector2 const&) const;				// substract 2 vectors
		Vector2			operator*(float) const;							// multiply all component by a value
		Vector2			operator/(float) const;							// divide all component by a value

		Vector2&		operator+=(Vector2 const);
		Vector2&		operator-=(Vector2 const);
		Vector2&		operator*=(float);
		Vector2&		operator/=(float);
						
		float&			operator[](int);								// return this vector component value
		float			operator[](int) const;							// return this vector component value

		float			magnitude() const;								// return vector magnitude
		float			magnitudeSquared() const;						// return square value of the vector magnitude
		float			dotProduct(Vector2 const&) const;				// return the dot product between 2 vector
		float			crossProduct(Vector2 const&) const;				// return the cross product between 2 vector

		bool			operator==(Vector2 const&) const;				// compare if 2 vector are equal
		bool			isLongerThan(Vector2 const&) const;				// return true if this vector magnitude is greater than the other
		bool			isShorterThan(Vector2 const&) const;			// return true if this vector magnitude is less than the other

		bool			isUnitVector() const;							// return true if this vector magnitude is 1

		Vector2&		normalize();									// scale this vector to have a magnitude of 1

		Radian			angleBetween(const Vector2& other) const;		// find the angle between 2 vector
		Vector2			projectOnto(const Vector2& other) const;		// project a vector onto an other
		Vector2			reflectOnto(const Vector2& other) const;		// reflect a vector onto an other

		static Vector2		lerp(Vector2 const&, Vector2 const&, float);

	private:
		float m_x;
		float m_y;
	};
}

#endif // !LIBMATH_VECTOR_VECTOR2_H_