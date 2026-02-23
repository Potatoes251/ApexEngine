#ifndef LIBMATH_MATRIX4VECTOR4OPERATION_H_
#define LIBMATH_MATRIX4VECTOR4OPERATION_H_

#include "Vector/Vector4.h"
#include "Matrix/Matrix4.h"

namespace LibMath
{
	Vector4 operator*(Matrix4&, Vector4&);
}

#endif // !LIBMATH_MATRIX4VECTOR4OPERATION_H_
