#ifndef LIBMATH_MATRIX4VECTOR4OPERATION_H_
#define LIBMATH_MATRIX4VECTOR4OPERATION_H_

namespace LibMath
{
	class Matrix4;
	class Vector4;
	Vector4 operator*(Matrix4&, Vector4&);
}

#endif // !LIBMATH_MATRIX4VECTOR4OPERATION_H_
