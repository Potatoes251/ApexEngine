#ifndef LIBMATH_VECTOR_VECTOR4_H_
#define LIBMATH_VECTOR_VECTOR4_H_

#include "Vector2.h"
#include "Vector3.h"

namespace LibMath
{
	class Vector4
	{
	public:
						Vector4();										// set all component to 0
		explicit		Vector4(float);									// set all component to the same value
						Vector4(float, float, float, float);			// set all component individually
						Vector4(Vector3 const&, float);					// copy all component
						Vector4(Vector4 const&);						// copy all component
						~Vector4() = default;

		Vector4&		operator=(Vector4 const&);

		Vector4			operator+(Vector4 const&) const;
		Vector4			operator*(float) const;
		Vector4			operator/(float) const;

		Vector4&		operator+=(Vector4 const&);
		Vector4&		operator*=(float);
		Vector4&		operator/=(float);

		float&			operator[](int);								// return this vector component value
		float			operator[](int) const;							// return this vector component value

		float			dot(Vector4 const&) const;						// return dot product result

		float			magnitude() const;								// return vector magnitude
		float			magnitudeSquared() const;						// return square value of the vector magnitude

		void			homogenize();

		bool			operator==(Vector4 const&) const;
		bool			operator!=(Vector4 const&) const;

	private:
		float m_x;
		float m_y;
		float m_z;
		float m_w;
	};
}

#ifdef LIBMATH_MATRIX_MATRIX4_H_
#include "Matrix4Vector4Operation.h"
#endif // LIBMATH_MATRIX_MATRIX4_H_

#endif // !LIBMATH_VECTOR_VECTOR4_H_
