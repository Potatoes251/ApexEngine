#include "LibMath/Angle.h"

LibMath::Degree::Degree() : m_value(0) {}

LibMath::Degree::Degree(float val) : m_value(val) {}

LibMath::Degree::Degree(Degree const& other) : m_value(other.m_value) {}

LibMath::Degree::operator LibMath::Radian() const
{
	return Radian(m_value * (3.14159265358979323f / 180.0f));
}

LibMath::Degree& LibMath::Degree::operator=(Degree const& other)
{
	m_value = other.m_value;
	return *this;
}

LibMath::Degree& LibMath::Degree::operator+=(Degree other)
{
	m_value += other.m_value;
	return *this;
}

LibMath::Degree& LibMath::Degree::operator-=(Degree other)
{
	m_value -= other.m_value;
	return *this;
}

LibMath::Degree& LibMath::Degree::operator*=(float val)
{
	m_value *= val;
	return *this;
}

LibMath::Degree& LibMath::Degree::operator/=(float val)
{
	m_value /= val;
	return *this;
}

void LibMath::Degree::wrap(bool boolean)
{
	while (m_value >= 360)
	{
		m_value -= 360;
	}
	while (m_value < 0)
	{
		m_value += 360;
	}
	if (boolean && m_value >= 180)
	{
		m_value -= 360;
	}
}

float LibMath::Degree::degree(bool boolean) const
{
	float val = m_value;
	while (val >= 360)
	{
		val -= 360;
	}
	while (val < 0)
	{
		val += 360;
	}
	if (boolean && val >= 180)
	{
		val -= 360;
	}
	return val;
}

float LibMath::Degree::radian(bool boolean) const
{
	return degree(boolean) * 3.1415927f / 180;
}

bool LibMath::operator==(Degree a1, Degree a2)
{
	return a1.degree() == a2.degree();
}

bool LibMath::operator==(Degree a1, Radian const& a2)
{
	return a1.raw() == a2.raw();
}

LibMath::Degree LibMath::operator-(Degree other)
{
	return Degree(-other.raw());
}

LibMath::Degree LibMath::operator+(Degree a1, Degree a2)
{
	return Degree(a1.raw() + a2.raw());
}

LibMath::Degree LibMath::operator-(Degree a1, Degree a2)
{
	return Degree(a1.raw() - a2.raw());
}

LibMath::Degree LibMath::operator*(Degree angle, float val)
{
	return Degree(angle.raw() * val);
}

LibMath::Degree LibMath::operator/(Degree angle, float val)
{
	return Degree(angle.raw() / val);
}

LibMath::Degree LibMath::Literal::operator""_deg(long double value)
{
	return Degree(static_cast<float>(value));
}

LibMath::Degree LibMath::Literal::operator""_deg(unsigned long long int value)
{
	return LibMath::Degree(static_cast<float>(value));
}




LibMath::Radian::Radian() : m_value(0) {}

LibMath::Radian::Radian(float val) : m_value(val) {}

LibMath::Radian::Radian(Radian const& other) : m_value(other.m_value) {}

LibMath::Radian::operator LibMath::Degree() const
{
	return Degree(m_value * 180.0f / 3.14159265358979323f);
}

LibMath::Radian& LibMath::Radian::operator=(Radian other)
{
	m_value = other.m_value;
	return *this;
}

LibMath::Radian& LibMath::Radian::operator+=(Radian other)
{
	m_value += other.m_value;
	return *this;
}

LibMath::Radian& LibMath::Radian::operator-=(Radian other)
{
	m_value -= other.m_value;
	return *this;
}

LibMath::Radian& LibMath::Radian::operator+=(float val)
{
	m_value += val;
	return *this;
}

LibMath::Radian& LibMath::Radian::operator-=(float val)
{
	m_value -= val;
	return *this;
}

LibMath::Radian& LibMath::Radian::operator*=(float val)
{
	m_value *= val;
	return *this;
}

LibMath::Radian& LibMath::Radian::operator/=(float val)
{
	m_value /= val;
	return *this;
}

// Wrap function
void LibMath::Radian::wrap(bool rangePi)
{
	m_value = rangePi ? wrapWithinRange(m_value, -g_Pi, g_Pi)
					: wrapWithinRange(m_value, 0.0f, g_twoPi);
}

float LibMath::Radian::degree(bool boolean) const
{
	return radian(boolean) * 180 / 3.14159265358979323f;
}

float LibMath::Radian::radian(bool rangePi) const
{
	float rad = wrapWithinRange(m_value, 0.0f, g_twoPi);
	return (rangePi && rad >= g_Pi) ? rad - g_twoPi : rad;
}

float LibMath::Radian::raw() const
{
	return m_value;
}

bool LibMath::operator==(Radian a1, Radian a2)
{
	return std::fabs(a1.radian() - a2.radian()) < g_epsilon;
}

bool LibMath::operator==(Radian a1, Degree const& a2)
{
	return a1.radian() == a2.radian();
}

LibMath::Radian LibMath::operator-(Radian other)
{
	return Radian(-other.raw());
}

LibMath::Radian LibMath::operator+(Radian a1, Radian a2)
{
	return Radian(a1.raw() + a2.raw());
}

LibMath::Radian LibMath::operator-(Radian a1, Radian a2)
{
	return Radian(a1.raw() - a2.raw());
}

LibMath::Radian LibMath::operator*(Radian angle, float val)
{
	return Radian(angle.raw() * val);
}

LibMath::Radian LibMath::operator/(Radian angle, float val)
{
	return Radian(angle.raw() / val);
}

LibMath::Radian LibMath::Literal::operator""_rad(long double value)
{
	return LibMath::Radian(static_cast<float>(value));
}

LibMath::Radian LibMath::Literal::operator""_rad(unsigned long long int value)
{
	return LibMath::Radian(static_cast<float>(value));
}


float wrapWithinRange(float value, float min, float max)
{
	float range = max - min;

	value = std::fmod(value - min, range);
	if (value < 0.0f)
	{
		value += range;
	}

	return value + min;

}