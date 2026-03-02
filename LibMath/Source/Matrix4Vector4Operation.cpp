#include "LibMath/Matrix4Vector4Operation.h"
#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector4.h"

LibMath::Vector4 LibMath::operator*(Matrix4& m , Vector4& vec)
{
	Vector4 product;
	for (int i = 0; i < 4; ++i)
	{
		product[i] = m[0][i] * vec[0] +
			m[1][i] * vec[1] +
			m[2][i] * vec[2] +
			m[3][i] * vec[3];
	}

	return product;
}
