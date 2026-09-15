#ifndef LIBMATH_ANGLE_H_
#define LIBMATH_ANGLE_H_

#define _USE_MATH_DEFINES
#include <cmath>
#include "Angle/Radian.h"
#include "Angle/Degree.h"
#include <limits>

constexpr float g_Pi = 3.141592653589723f;
constexpr float g_twoPi = 2 * g_Pi;
constexpr float g_epsilon = std::numeric_limits<float>::epsilon();;

float wrapWithinRange(float value, float min, float max);

#endif // !LIBMATH_ANGLE_H_
