#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Quaternion.h"
#include "LibMath/Angle/Radian.h"

namespace LibMath
{
	class Transform
	{
	public:
		Transform() = default;
		Transform(Transform const& other);
		Transform& operator=(Transform const other);

		~Transform() = default;

		Quaternion const&	getRotation() const { return m_rotation; }
		Vector3	const&		getPosition() const { return m_position; }
		Vector3	const&		getScale() const { return m_scale; }
		Vector3				getForward() const;	

		Vector3		getEulerRotation() const;
		void		setEulerRotation(Vector3 euler);
		
		void	setRotation(Radian pitch, Radian yaw, Radian roll) { m_rotation = Quaternion(pitch, yaw, roll); }
		void	setRotation(Quaternion rotation) { m_rotation = rotation; }
		void	setPosition(Vector3 position) { m_position = position; }
		void	setScale(Vector3 scale) { m_scale = scale; }

		Transform inversed() const;

		operator Matrix4() const;

		// parent * child
		Transform operator*(Transform const& other) const;

	private:
		Quaternion m_rotation = Quaternion(0, 0, 0, 1);
		Vector3 m_position = Vector3(0);
		Vector3 m_scale = Vector3(1);
	};
}

#endif // !TRANSFORM_H

