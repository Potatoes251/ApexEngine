#ifndef LIBMATH_MATRIX_MATRIX4_H_
#define LIBMATH_MATRIX_MATRIX4_H_

#include "Matrix3.h"
#include "../Vector/Vector3.h"
#include "../Angle/Radian.h"

namespace LibMath
{
	class Matrix4
	{
	public:
		Matrix4();														// default constructor
		Matrix4(float);													// construct a matrix with diagonal
		Matrix4(float, float, float, float,								// construct a matrix with all components
			float, float, float, float,
			float, float, float, float,
			float, float, float, float);

		const float*			data() const;

		Matrix4&		operator=(Matrix4 const);						// copy asignement

		float*			operator[](int);

		Matrix4			operator+(Matrix4 const) const;
		Matrix4			operator*(float const) const;
		Vector4			operator*(Vector4 const) const;
		Matrix4			operator*(Matrix4 const) const;

		bool			operator==(Matrix4 const) const;

		Matrix3x3		getMinor(int, int);			// get the 3X3 minor matrix by excluding the given row and column

		void			transpose();				// transpose the matrix
		float			determinant();				// return the determinant of the matrix
		void			minors();					// change the matrix into a minor matrix
		void			cofactors();				// change the matrix into a cofactor matrix
		void			adjugate();					// change the matrix into the adjugate matrix
		void			inverse();					// inverse the matrix

		static Matrix4		identity();												// create an identity matrix

		static Matrix4		createScale(Vector3);									// return a scaling matrix

		static Matrix4		createTranslation(Vector3);							// return a translation matrix
		static Matrix4		createRotation(Vector3, Radian);					// return a rotation matrix
		static Matrix4		createTransform(Vector3, Vector3, Radian, Vector3);	// return a transformation matrix

		static Matrix4		createRotationX(Radian);
		static Matrix4		createRotationY(Radian);
		static Matrix4		createRotationZ(Radian);

		static Matrix4		perspective(LibMath::Radian fovRadian, float aspectRatio, float near, float far);
		static Matrix4		lookAt(Vector3 eye, Vector3 target, Vector3 up);
		static Matrix4		orthogonal(float left, float right, float bottom, float top, float near, float far);

	private:
		float m_values[4][4];
	};
}

#ifdef LIBMATH_VECTOR_VECTOR4_H_
#include "LibMath/Matrix4Vector4Operation.h"
#endif // LIBMATH_MATRIX_H_

#endif // !LIBMATH_MATRIX_MATRIX4_H_
