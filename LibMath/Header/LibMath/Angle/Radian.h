#ifndef LIBMATH_ANGLE_RADIAN_H_
#define LIBMATH_ANGLE_RADIAN_H_

namespace LibMath
{
	class Degree;

	class Radian
	{
	public:
					Radian();
		explicit	Radian(float);				// explicit so no ambiguous / implicit conversion from float to angle can happen
					Radian(Radian const&);
					~Radian() = default;

		operator	Degree() const;				// Degree angle = Radian{0.5};		// implicit conversion from Radian to Degree

		Radian&		operator=(Radian);
		Radian&		operator+=(Radian);			// Radian angle += Radian{0.5};
		Radian&		operator-=(Radian);			// Radian angle -= Radian{0.5};
		Radian&		operator+=(float);			// Radian angle += Radian{0.5};
		Radian&		operator-=(float);			// Radian angle -= Radian{0.5};
		Radian&		operator*=(float);			// Radian angle *= 3;
		Radian&		operator/=(float);			// Radian angle /= 3;

		void		wrap(bool = false);			// true -> limit m_value to range [-pi, pi[		// false -> limit m_value to range [0, 2 pi[

		float		degree(bool = false) const;	// return angle in degree	// true -> return value in range [-180, 180[	// false -> return value in range [0, 360[
		float		radian(bool = true) const;	// return angle in radian	// true -> return value in range [-pi, pi[		// false -> return value in range [0, 2 pi[
		float		raw() const;				// return m_angle

	private:
		float m_value;
	};

	bool	operator==(Radian, Radian);			// bool isEqual = Radian{0.5} == Radian{0.5};	// true
	bool	operator==(Radian, Degree const&);	// bool isEqual = Radian{0.5} == Degree{60};	// false

	Radian	operator-(Radian);					// Degree angle = - Radian{0.5};				// Radian{-0.5}

	Radian	operator+(Radian, Radian);			// Radian angle = Radian{0.5} + Radian{0.5};	// Radian{1}
	Radian	operator-(Radian, Radian);			// Radian angle = Radian{0.5} - Radian{0.5};	// Radian{0}
	Radian	operator*(Radian, float);			// Radian angle = Radian{0.5} * 3;				// Radian{1.5}
	Radian	operator/(Radian, float);			// Radian angle = Radian{0.5} / 3;				// Radian{0.166...}

	inline namespace Literal
	{
		LibMath::Radian operator""_rad(long double);			// Radian angle = 0.5_rad;
		LibMath::Radian operator""_rad(unsigned long long int);	// Radian angle = 1_rad;
	}
}

#endif // !LIBMATH_ANGLE_RADIAN_H_
