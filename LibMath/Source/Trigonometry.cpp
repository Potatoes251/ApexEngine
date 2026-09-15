
#include <cmath>

#include "LibMath/Trigonometry.h"
#include <iostream>


float LibMath::sin(Radian angle)
{
	return std::sin(static_cast<float>(angle.radian()));
}

float LibMath::cos(Radian angle)
{
	return std::cos(static_cast<float>(angle.radian()));
}

float LibMath::tan(Radian angle)
{
	return std::tan(static_cast<float>(angle.radian()));
}

LibMath::Radian LibMath::asin(float sin)
{
	return Radian(std::asin(sin));
}

LibMath::Radian LibMath::acos(float cos)
{
	return Radian(std::acos(cos));
}

LibMath::Radian LibMath::atan(float tan)
{
	return Radian(std::atan(tan));
}

LibMath::Radian LibMath::atan(float y, float x)
{
	return Radian(std::atan2(y, x));
}
