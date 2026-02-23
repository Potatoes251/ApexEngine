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
    if (value < 0)
    {
        return 0.f;
    }
    if (value == 0.f || value == 1.f)
    {
        return value;
    }
    int	maxIterations = 100;        // set a maximum to the amount of iterations possible

    int iterations = 0;
    float root = value > 0 ? value * 0.5f : value;

    while (std::fabs(root * root - value) > g_tolerance &&
        iterations < maxIterations)
    {
        root = static_cast<float>(0.5 * (root + value / root));
        ++iterations;
    }

    return root;
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
