#ifndef LIBMATH_QUATERNION_H_
#define LIBMATH_QUATERNION_H_

#include "Matrix.h"

namespace LibMath
{
	class Radian;
	class Vector3;

	class Quaternion
	{
	public:
		Quaternion();										// set all component to 0
		Quaternion(float, float, float, float);				// set all component individually as x, y, z, w
		Quaternion(Quaternion const&);						// copy all component
		Quaternion(Radian, Radian, Radian);					// create rotation from euler angles
		Quaternion(Radian , Vector3);						// create rotation from an angle and an axis
		~Quaternion() = default;

		float				magnitude() const;
		float				magnitudeSquared() const;
		bool				isUnit() const;
		Quaternion			normalized() const;
		Quaternion			conjugated() const;
		Quaternion			inversed() const;
		Quaternion&			normalize();
		Quaternion&			conjugate();
		Quaternion&			inverse();
		// assumes a unit quaternion
		Vector3 			rotate(Vector3 const&) const;
		// assume two unit quaternions
		float				angleBetween(Quaternion const&) const;

		Vector3				toEuler() const;

		float&			operator[](int);					// return this quaternion component value
		float			operator[](int) const;				// return this quaternion component value

		bool			operator==(Quaternion const&) const;
		bool			operator!=(Quaternion const&) const;

		Quaternion		operator+(Quaternion const&) const;
		Quaternion		operator*(Quaternion const&) const;
		Quaternion		operator*(float) const;

		// assumes a unit quaternion
		operator Matrix4() const;

		static Quaternion	slerp(Quaternion, Quaternion, float);
		static Quaternion	identity();						// return { 0, 0, 0, 1 }

	private:
		float m_x;
		float m_y;
		float m_z;
		float m_w;
	};
}

#endif // !LIBMATH_QUATERNION_H_
