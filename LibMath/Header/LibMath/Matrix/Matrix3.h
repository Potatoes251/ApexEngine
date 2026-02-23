#ifndef LIBMATH_MATRIX_MATRIX3_H_
#define LIBMATH_MATRIX_MATRIX3_H_

#include "Matrix2.h"
#include "../Vector/Vector2.h"
#include "../Angle/Radian.h"

namespace LibMath
{
	class Matrix3x3
	{
	public:
		Matrix3x3();														// default constructor
		Matrix3x3(float);													// construct a matrix with diagonal
		Matrix3x3(float, float, float,										// construct a matrix with all components
			float, float, float,
			float, float, float);

		float*			data();

		Matrix3x3&		operator=(Matrix3x3 const);							// copy asignement

		float*			operator[](int);

		Matrix3x3		operator+(Matrix3x3 const) const;
		Matrix3x3		operator*(float const) const;
		Vector3			operator*(Vector3 const) const;
		Matrix3x3		operator*(Matrix3x3 const) const;

		bool			operator==(Matrix3x3 const) const;

		Matrix2x2		getMinor(int, int);			// get the 2X2 minor matrix by excluding the given row and column

		void			transpose();				// transpose the matrix
		float			determinant();				// return the determinant of the matrix
		void			minors();					// change the matrix into a minor matrix
		void			cofactors();				// change the matrix into a cofactor matrix
		void			adjugate();					// change the matrix into the adjugate matrix
		void			inverse();					// inverse the matrix

		Matrix3x3&		identity();												// create an identity matrix

		Matrix3x3&		createScale(Vector2);									// return a scaling matrix

		static Matrix3x3		createTranslation(Vector2);					// return a translation matrix
		static Matrix3x3		createRotation(Point2D, Radian);			// return a rotation matrix
		static Matrix3x3		createTransform(Vector2, Radian, Vector2);	// return a transformation matrix

		static Matrix3x3		createRotationX(Radian);
		static Matrix3x3		createRotationY(Radian);
		static Matrix3x3		createRotationZ(Radian);


	private:
		float m_values[3][3];
	};
}

#endif // !LIBMATH_MATRIX_MATRIX3_H_
