#ifndef LIBMATH_ARITHMETIC_H_
#define LIBMATH_ARITHMETIC_H_

constexpr float g_tolerance = 1e-6f;

namespace LibMath
{
	bool almostEqual(float, float);		// Return if two floating value are similar enought to be considered equal

	float ceiling(float);				// Return lowest integer value higher or equal to parameter
	float clamp(float, float, float);	// Return parameter limited by the given range
	float lerp(float, float, float);	// Return lerp between the a and b 
	float floor(float);					// Return highest integer value lower or equal to parameter
	float squareRoot(float);			// Return square root of parameter
	float wrap(float, float, float);	// Return parameter as value inside the given range
}

#endif // !LIBMATH_ARITHMETIC_H_
