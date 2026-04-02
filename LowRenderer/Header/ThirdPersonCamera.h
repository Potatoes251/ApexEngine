#ifndef THIRD_PERSON_CAMERA
#define THIRD_PERSON_CAMERA

#include "Camera.h"

namespace Apex::Rendering
{
	class ThirdPersonCamera : public Camera
	{
	public:
		ThirdPersonCamera(LibMath::Vector3 target, float distance);

		void ProcessMouse(double dx, double dy, float sensitivity = 0.1f) override;

		LibMath::Vector3 GetTarget() const { return m_target; }
		LibMath::Vector3 GetFront() const override { return m_target - m_position; }

	private:
		void UpdateMatrices() override;

		LibMath::Vector3 m_target;
		float m_distance;
	};
}

#endif // !FIRST_PERSON_CAMERA

