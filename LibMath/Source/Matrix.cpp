#include "LibMath/Matrix.h"
#include "LibMath/Trigonometry.h"
#include "LibMath/Quaternion.h"


LibMath::Matrix2x2::Matrix2x2()
{
	m_values[0][0] = 0;
	m_values[0][1] = 0;
	m_values[1][0] = 0;
	m_values[1][1] = 0;
}

LibMath::Matrix2x2::Matrix2x2(float diagonal)
{
	m_values[0][0] = diagonal;
	m_values[0][1] = 0;
	m_values[1][0] = 0;
	m_values[1][1] = diagonal;
}

LibMath::Matrix2x2::Matrix2x2(float val1, float val2, float val3, float val4)
{
	m_values[0][0] = val1;
	m_values[0][1] = val2;
	m_values[1][0] = val3;
	m_values[1][1] = val4;
}

LibMath::Matrix2x2& LibMath::Matrix2x2::operator=(Matrix2x2 const other)
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2;j++)
		{
			m_values[i][j] = other.m_values[i][j];
		}
	}
	return *this;
}

float* LibMath::Matrix2x2::operator[](int index)
{
	return m_values[index];
}

LibMath::Matrix2x2 LibMath::Matrix2x2::operator+(Matrix2x2 const other) const
{
	Matrix2x2 sum;
	sum[0][0] = m_values[0][0] + other.m_values[0][0];
	sum[0][1] = m_values[0][1] + other.m_values[0][1];
	sum[1][0] = m_values[1][0] + other.m_values[1][0];
	sum[1][1] = m_values[1][1] + other.m_values[1][1];

	return sum;
}

LibMath::Matrix2x2 LibMath::Matrix2x2::operator*(float const val) const
{
	return Matrix2x2 (
		m_values[0][0] * val,
		m_values[0][1] * val,
		m_values[1][0] * val,
		m_values[1][1] * val);

}

LibMath::Vector2 LibMath::Matrix2x2::operator*(Vector2 const v) const
{
	float x = m_values[0][0] * v[0] + m_values[1][0] * v[1];
	float y = m_values[0][1] * v[0] + m_values[1][1] * v[1];

	return Vector2(x, y);
}

LibMath::Matrix2x2 LibMath::Matrix2x2::operator*(Matrix2x2 const other) const
{
	Matrix2x2 product;
	for (int col = 0; col < 2; col++)
	{
		for (int row = 0; row < 2; row++)
		{
			product[col][row] =
				m_values[0][row] * other.m_values[col][0] +
				m_values[1][row] * other.m_values[col][1];
		}
	}
	return product;
}

bool LibMath::Matrix2x2::operator==(Matrix2x2 const other) const
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2;j++)
		{
			if (m_values[i][j] != other.m_values[i][j])
			{
				return false;
			}
		}
	}
	return true;
}

void LibMath::Matrix2x2::transpose()
{
	float temp = m_values[0][1];
	m_values[0][1] = m_values[1][0];
	m_values[1][0] = temp;
}

float LibMath::Matrix2x2::determinant()
{
	return m_values[0][0] * m_values[1][1] - m_values[1][0] * m_values[0][1];
}

void LibMath::Matrix2x2::minors()
{
	float temp = m_values[0][0];
	m_values[0][0] = m_values[1][1];
	m_values[1][1] = temp;

	temp = m_values[0][1];
	m_values[0][1] = m_values[1][0];
	m_values[1][0] = temp;
}

void LibMath::Matrix2x2::cofactors()
{
	float temp = m_values[0][0];
	m_values[0][0] = m_values[1][1];
	m_values[1][1] = temp;

	temp = m_values[0][1];
	m_values[0][1] = -m_values[1][0];
	m_values[1][0] = -temp;
}

void LibMath::Matrix2x2::adjugate()
{
	float temp = m_values[0][0];
	m_values[0][0] = m_values[1][1];
	m_values[1][1] = temp;

	m_values[0][1] = -m_values[0][1];
	m_values[1][0] = -m_values[1][0];
}

void LibMath::Matrix2x2::inverse()
{
	float det = determinant();

	if (det == 0)
	{
		return;
	}

	adjugate();

	m_values[0][0] /= det;
	m_values[0][1] /= det;
	m_values[1][0] /= det;
	m_values[1][1] /= det;
}

LibMath::Matrix2x2& LibMath::Matrix2x2::identity()
{
	m_values[0][0] = 1;
	m_values[0][1] = 0;
	m_values[1][0] = 0;
	m_values[1][1] = 1;

	return *this;
}

LibMath::Matrix2x2& LibMath::Matrix2x2::createScale(Vector2 v)
{
	m_values[0][0] = v[0];
	m_values[1][1] = v[1];

	return *this;
}

LibMath::Matrix2x2 LibMath::Matrix2x2::createRotation(Radian angle)
{
	Matrix2x2 rotation;
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);
	rotation[0][0] = cosAngle;
	rotation[0][1] = sinAngle;
	rotation[1][0] = -sinAngle;
	rotation[1][1] = cosAngle;

	return rotation;
}

LibMath::Matrix2x2 LibMath::Matrix2x2::createTransform(Radian angle, Vector2 scale)
{
	Matrix2x2 rotation = createRotation(angle);

	Matrix2x2 scaler;
	scaler.createScale(scale);

	return scaler * rotation;
}


LibMath::Matrix3x3::Matrix3x3()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3;j++)
		{
			m_values[i][j] = 0;
		}
	}
}

LibMath::Matrix3x3::Matrix3x3(float val)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (i == j)
			{
				m_values[i][j] = val;
			}
			else
			{
				m_values[i][j] = 0;
			}
		}
	}
}

LibMath::Matrix3x3::Matrix3x3(
	float val1, float val2, float val3,
	float val4, float val5, float val6,
	float val7, float val8, float val9)
{
	m_values[0][0] = val1;  // col 0, row 0
	m_values[0][1] = val2;  // col 0, row 1
	m_values[0][2] = val3;  // col 0, row 2

	m_values[1][0] = val4;  // col 1, row 0
	m_values[1][1] = val5;  // col 1, row 1
	m_values[1][2] = val6;  // col 1, row 2

	m_values[2][0] = val7;  // col 2, row 0
	m_values[2][1] = val8;  // col 2, row 1
	m_values[2][2] = val9;  // col 2, row 2
}

float* LibMath::Matrix3x3::data()
{
	return &m_values[0][0];
}


LibMath::Matrix3x3& LibMath::Matrix3x3::operator=(Matrix3x3 const other)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3;j++)
		{
			m_values[i][j] = other.m_values[i][j];
		}
	}
	return *this;
}

float* LibMath::Matrix3x3::operator[](int index)
{
	return m_values[index];
}

LibMath::Matrix3x3 LibMath::Matrix3x3::operator+(Matrix3x3 const other) const
{
	Matrix3x3 sum;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3;j++)
		{
			sum[i][j] = m_values[i][j] + other.m_values[i][j];
		}
	}

	return sum;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::operator*(float const val) const
{
	Matrix3x3 res{};
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3;j++)
		{
			res.m_values[i][j] = m_values[i][j] * val;
		}
	}
	return res;
}

LibMath::Vector3 LibMath::Matrix3x3::operator*(Vector3 const vec) const
{
	Vector3 result;

	for (int i = 0; i < 3; ++i)
	{
		result[i] = m_values[0][i] * vec[0] +
			m_values[1][i] * vec[1] +
			m_values[2][i] * vec[2];
	}

	return result;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::operator*(Matrix3x3 const other) const
{
	Matrix3x3 product;
	for (int col = 0; col < 3; ++col)       // loop over result columns
	{
		for (int row = 0; row < 3; ++row)   // loop over result rows
		{
			product.m_values[col][row] =
				m_values[0][row] * other.m_values[col][0] +
				m_values[1][row] * other.m_values[col][1] +
				m_values[2][row] * other.m_values[col][2];
		}
	}

	return product;
}

bool LibMath::Matrix3x3::operator==(Matrix3x3 const other) const
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (std::fabs(m_values[i][j] - other.m_values[i][j]) >= g_epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

LibMath::Matrix2x2 LibMath::Matrix3x3::getMinor(int rowToExclude, int colToExclude)
{
	Matrix2x2 minor;
	int minorCol = 0;
	for (int col = 0; col < 3; ++col)
	{
		if (col == colToExclude)
			continue;

		int minorRow = 0;
		for (int row = 0; row < 3; ++row)
		{
			if (row == rowToExclude)
				continue;

			minor[minorCol][minorRow] = m_values[col][row];
			++minorRow;
		}
		++minorCol;
	}
	return minor;
}

void LibMath::Matrix3x3::transpose()
{
	float temp;
	for (int col = 0; col < 3; ++col)
	{
		for (int row = col + 1; row < 3; ++row)
		{
			temp = m_values[col][row];
			m_values[col][row] = m_values[row][col];
			m_values[row][col] = temp;
		}
	}
}

float LibMath::Matrix3x3::determinant()
{
	return (m_values[0][0] * getMinor(0, 0).determinant()
		- m_values[1][0] * getMinor(0, 1).determinant()
		+ m_values[2][0] * getMinor(0, 2).determinant());
}

void LibMath::Matrix3x3::minors()
{
	Matrix3x3 copy = *this;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_values[i][j] = copy.getMinor(i, j).determinant();
		}
	}
}

void LibMath::Matrix3x3::cofactors()
{
	minors();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if ((i + j) % 2 == 1)
			{
				m_values[i][j] *= -1;
			}
		}
	}
}

void LibMath::Matrix3x3::adjugate()
{
	cofactors();
	transpose();
}

void LibMath::Matrix3x3::inverse()
{
	float det = determinant();

	if (det == 0.0f)  // Prevent division by zero
	{
		throw std::runtime_error("Matrix is singular and cannot be inverted.");
	}

	adjugate();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_values[i][j] /= det;
		}
	}
}

LibMath::Matrix3x3& LibMath::Matrix3x3::identity()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (i == j)
			{
				m_values[i][j] = 1;
			}
			else
			{
				m_values[i][j] = 0;
			}
		}
	}
	return *this;
}

LibMath::Matrix3x3& LibMath::Matrix3x3::createScale(Vector2 v)
{
	identity();
	m_values[0][0] = v[0];
	m_values[1][1] = v[1];
	m_values[2][2] = 1.f;

	return *this;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::createTranslation(Vector2 v)
{
	Matrix3x3 TransMatrix{};
	TransMatrix.identity();
	TransMatrix[2][0] = v[0];
	TransMatrix[2][1] = v[1];

	return TransMatrix;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::createRotation(Point2D point, Radian angle)
{
	Matrix3x3 toOrigin;
	toOrigin.identity();
	toOrigin[2][0] = -point.getX();
	toOrigin[2][1] = -point.getY();

	Matrix3x3 rotation;
	rotation.identity();
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);
	rotation[0][0] = cosAngle;
	rotation[0][1] = sinAngle;
	rotation[1][0] = -sinAngle;
	rotation[1][1] = cosAngle;

	Matrix3x3 toPivot;
	toPivot.identity();
	toPivot[2][0] = point.getX();
	toPivot[2][1] = point.getY();

	return toPivot * rotation * toOrigin;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::createTransform(Vector2 translation, Radian rotation, Vector2 scale) 
{
	Matrix3x3 transformMatrix;
	transformMatrix.identity();

	float cosTheta = cos(rotation);
	float sinTheta = sin(rotation);

	transformMatrix.m_values[0][0] = cosTheta * scale[0];   // col 0, row 0
	transformMatrix.m_values[0][1] = sinTheta * scale[0];   // col 0, row 1

	transformMatrix.m_values[1][0] = -sinTheta * scale[1];  // col 1, row 0
	transformMatrix.m_values[1][1] = cosTheta * scale[1];   // col 1, row 1

	transformMatrix.m_values[2][0] = translation[0];        // col 2, row 0
	transformMatrix.m_values[2][1] = translation[1];        // col 2, row 1

	transformMatrix.m_values[2][2] = 1.0f;

	return transformMatrix;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::createRotationX(Radian angle)
{
	Matrix3x3 rotation;
	rotation.identity();

	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	rotation[1][1] = cosAngle;
	rotation[1][2] = sinAngle;
	rotation[2][1] = -sinAngle;
	rotation[2][2] = cosAngle;

	return rotation;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::createRotationY(Radian angle)
{
	Matrix3x3 rotation;
	rotation.identity();

	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	rotation[0][0] = cosAngle;
	rotation[2][0] = -sinAngle;
	rotation[0][2] = sinAngle;
	rotation[2][2] = cosAngle;

	return rotation;
}

LibMath::Matrix3x3 LibMath::Matrix3x3::createRotationZ(Radian angle)
{
	Matrix3x3 rotation;
	rotation.identity();

	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	rotation[0][0] = cosAngle;
	rotation[0][1] = -sinAngle;
	rotation[1][0] = sinAngle;
	rotation[1][1] = cosAngle;

	return rotation;
}


LibMath::Matrix4::Matrix4()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4;j++)
		{
			m_values[i][j] = 0;
		}
	}
}

LibMath::Matrix4::Matrix4(float val)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
			{
				m_values[i][j] = val;
			}
			else
			{
				m_values[i][j] = 0;
			}
		}
	}
}

LibMath::Matrix4::Matrix4(float val1, float val2, float val3, float val4,
						float val5, float val6, float val7, float val8,
						float val9, float val10, float val11, float val12,
						float val13, float val14, float val15, float val16)
{
	m_values[0][0] = val1;   // col 0, row 0
	m_values[0][1] = val2;   // col 0, row 1
	m_values[0][2] = val3;   // col 0, row 2
	m_values[0][3] = val4;   // col 0, row 3

	m_values[1][0] = val5;   // col 1, row 0
	m_values[1][1] = val6;   // col 1, row 1
	m_values[1][2] = val7;   // col 1, row 2
	m_values[1][3] = val8;   // col 1, row 3

	m_values[2][0] = val9;   // col 2, row 0
	m_values[2][1] = val10;  // col 2, row 1
	m_values[2][2] = val11;  // col 2, row 2
	m_values[2][3] = val12;  // col 2, row 3
								
	m_values[3][0] = val13;  // col 3, row 0
	m_values[3][1] = val14;  // col 3, row 1
	m_values[3][2] = val15;  // col 3, row 2
	m_values[3][3] = val16;  // col 3, row 3
}

const float* LibMath::Matrix4::data() const
{
	return &m_values[0][0];
}

LibMath::Matrix4& LibMath::Matrix4::operator=(Matrix4 const other)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4;j++)
		{
			m_values[i][j] = other.m_values[i][j];
		}
	}
	return *this;
}

float* LibMath::Matrix4::operator[](int index)
{
	return m_values[index];
}

LibMath::Matrix4 LibMath::Matrix4::operator+(Matrix4 const other) const
{
	Matrix4 sum;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4;j++)
		{
			sum[i][j] = m_values[i][j] + other.m_values[i][j];
		}
	}

	return sum;
}

LibMath::Matrix4 LibMath::Matrix4::operator*(float const val) const
{
	Matrix4 product;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4;j++)
		{
			product[i][j] = m_values[i][j] * val;
		}
	}

	return product;
}

LibMath::Vector4 LibMath::Matrix4::operator*(Vector4 const vec) const
{
	Vector4 product;
	for (int i = 0; i < 4; ++i)
	{
		product[i] = m_values[0][i] * vec[0] +
			m_values[1][i] * vec[1] +
			m_values[2][i] * vec[2] +
			m_values[3][i] * vec[3];
	}

	return product;
}

LibMath::Matrix4 LibMath::Matrix4::operator*(Matrix4 const other) const
{
	Matrix4 product;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4;j++)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				sum += m_values[k][j] * other.m_values[i][k];
			}
			product[i][j] = sum;
		}
	}
	return product;
}

bool LibMath::Matrix4::operator==(Matrix4 const other) const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4;j++)
		{
			if (std::fabs(m_values[i][j] - other.m_values[i][j]) >= g_epsilon)
			{
				return false;
			}
		}
	}
	return true;
}

LibMath::Matrix3x3 LibMath::Matrix4::getMinor(int row, int column)
{
	Matrix3x3 minor;
	int n = 0;
	int m = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (i == row) continue;

		for (int j = 0; j < 4; ++j)
		{
			if (j == column) continue;

			minor[m][n] = m_values[j][i];
			++m;
		}
		m = 0;
		++n;
	}
	return minor;
}

void LibMath::Matrix4::transpose()
{
	float temp;
	for (int i = 0; i < 4; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			if (i != j)
			{
				temp = m_values[i][j];
				m_values[i][j] = m_values[j][i];
				m_values[j][i] = temp;
			}
		}
	}
}

float LibMath::Matrix4::determinant()
{
	return m_values[0][0] * getMinor(0, 0).determinant()
		- m_values[1][0] * getMinor(0, 1).determinant()
		+ m_values[2][0] * getMinor(0, 2).determinant()
		- m_values[3][0] * getMinor(0, 3).determinant();
}

void LibMath::Matrix4::minors()
{
	Matrix4 copy = *this;
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			m_values[row][col] = copy.getMinor(row, col).determinant();
		}
	}
}

void LibMath::Matrix4::cofactors()
{
	minors();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if ((i + j) % 2 == 1)
			{
				m_values[i][j] *= -1;
			}
		}
	}
}

void LibMath::Matrix4::adjugate()
{
	cofactors();
	//transpose();
}

void LibMath::Matrix4::inverse()
{
	float det = determinant();

	if (det == 0.0f)  // Prevent division by zero
	{
		throw std::runtime_error("Matrix is singular and cannot be inverted.");
	}

	adjugate();

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_values[j][i] /= det;
		}
	}
}

LibMath::Matrix4 LibMath::Matrix4::identity()
{
	return Matrix4(1.f);
}

LibMath::Matrix4 LibMath::Matrix4::createScale(Vector3 v)
{
	Matrix4 scale;
	scale[0][0] = v[0];
	scale[1][1] = v[1];
	scale[2][2] = v[2];
	scale[3][3] = 1.f;

	return scale;
}

LibMath::Matrix4 LibMath::Matrix4::createTranslation(Vector3 vec)
{
	Matrix4 TransMatrix = identity();
	TransMatrix[3][0] = vec[0];
	TransMatrix[3][1] = vec[1];
	TransMatrix[3][2] = vec[2];

	return TransMatrix;
}

LibMath::Matrix4 LibMath::Matrix4::createRotation(Vector3 axis, Radian angle)
{
	axis.normalize();

	float cosTheta = cos(angle);
	float sinTheta = sin(angle);
	float k = 1.0f - cosTheta;

	float x = axis[0];
	float y = axis[1];
	float z = axis[2];

	return Matrix4(
		cosTheta + x * x * k,		y * x * k + z * sinTheta,	z * x * k - y * sinTheta,	0.0f,
		x * y * k - z * sinTheta,	cosTheta + y * y * k,		z * y * k + x * sinTheta,	0.0f,
		x * z * k + y * sinTheta,	y * z * k - x * sinTheta,	cosTheta + z * z * k,		0.0f,
		0.0f,						0.0f,						0.0f,						1.0f
	);
}

LibMath::Matrix4 LibMath::Matrix4::createTransform(Vector3 position, Vector3 axis, Radian angle, Vector3 scale)
{
	Matrix4 rotationMatrix = createRotation(axis, angle);

	return Matrix4(
		scale[0] * rotationMatrix[0][0],	scale[0] * rotationMatrix[0][1],	scale[0] * rotationMatrix[0][2],	0.f,
		scale[1] * rotationMatrix[1][0],	scale[1] * rotationMatrix[1][1],	scale[1] * rotationMatrix[1][2],	0.f,
		scale[2] * rotationMatrix[2][0],	scale[2] * rotationMatrix[2][1],	scale[2] * rotationMatrix[2][2],	0.f,
		position[0],						position[1],						position[2],						1.f
	);
}

LibMath::Matrix4 LibMath::Matrix4::createRotationX(Radian angle)
{
	Matrix4 rotation = identity();

	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	rotation[1][1] = cosAngle;
	rotation[1][2] = sinAngle;
	rotation[2][1] = -sinAngle;
	rotation[2][2] = cosAngle;

	return rotation;
}

LibMath::Matrix4 LibMath::Matrix4::createRotationY(Radian angle)
{
	Matrix4 rotation = identity();

	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	rotation[0][0] = cosAngle;
	rotation[2][0] = sinAngle;
	rotation[0][2] = -sinAngle;
	rotation[2][2] = cosAngle;

	return rotation;
}

LibMath::Matrix4 LibMath::Matrix4::createRotationZ(Radian angle)
{
	Matrix4 rotation = identity();

	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	rotation[0][0] = cosAngle;
	rotation[0][1] = sinAngle;
	rotation[1][0] = -sinAngle;
	rotation[1][1] = cosAngle;

	return rotation;
}

LibMath::Matrix4 LibMath::Matrix4::perspective(LibMath::Radian fovRadian, float aspectRatio, float near, float far)
{
	float tanFov = tan((fovRadian / 2));

	return Matrix4(
		1 / (tanFov * aspectRatio),		0.f,			0.f,								0.f,
		0.f,							1 / tanFov,		0.f,								0.f,
		0.f,							0.f,			(far + near) / (near - far),		-1.f,
		0.f,							0.f,			(2 * far * near) / (near - far),	0.f
	);
}

LibMath::Matrix4 LibMath::Matrix4::lookAt(Vector3 eye, Vector3 target, Vector3 up)
{
	Vector3 zAxis = (target - eye);
	zAxis.normalize();

	// Compute the right vector (x-axis)
	Vector3 xAxis = up.cross(zAxis);
	xAxis.normalize();

	// Recompute the true up vector (y-axis)
	Vector3 yAxis = zAxis.cross(xAxis);

	return LibMath::Matrix4(
		-xAxis[0],				yAxis[0],			-zAxis[0], .0f,
		-xAxis[1],				yAxis[1],			-zAxis[1], .0f,
		-xAxis[2],				yAxis[2],			-zAxis[2], .0f,
		xAxis.dot(eye),		-yAxis.dot(eye),	zAxis.dot(eye), 1.0f);
}

LibMath::Matrix4 LibMath::Matrix4::orthogonal(float left, float right, float bottom, float top, float near, float far)
{
	return Matrix4(
		2 / (right - left),					0.f,								0.f,							0.f,
		0.f,								2 / (top - bottom),					0.f,							0.f,
		0.f,								0.f,								2 / (near - far),				0.f,
		(right + left) / (left - right),	(top + bottom) / (bottom - top),	(far + near) / (near - far),	1.f
	);
}
