#define _USE_MATH_DEFINES
#include <cmath>
#include "iostream"
#include "LibMath/Arithmetic.h"

bool LibMath::almostEqual(float val1, float val2)
{
    return fabs(val1 - val2) < g_tolerance;
}

float LibMath::ceiling(float value)
{
    int intPart = static_cast<int> (value);
    return (value == static_cast<float>(intPart)) ? value : ++intPart;
}

float LibMath::clamp(float value, float lowRange, float highRange)
{
    return (value < lowRange) ? lowRange : ((value > highRange) ? highRange : value);
}

float LibMath::lerp(float start, float end, float alpha)
{
    return start + (end - start) * alpha;
}

float LibMath::floor(float value)
{
    int intPart = static_cast<int> (value);
    return (value == static_cast<float>(intPart)) ? value : intPart;
}

float LibMath::squareRoot(float value)
{
    return std::sqrt(value);
}

float LibMath::wrap(float value, float lowRange, float highRange)
{
    float rangeSize = highRange - lowRange;

    if (rangeSize < g_tolerance)
    {
        return lowRange;
    }

    while (value >= highRange)
    {
        value -= rangeSize;
    }
    while (value < lowRange)
    {
        value += rangeSize;
    }

    return value;
}
