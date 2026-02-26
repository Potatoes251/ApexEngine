#ifndef CAMERA
#define CAMERA

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"

namespace Apex::Rendering
{
	class Camera
	{
	public:
		Camera(LibMath::Vector3 position = LibMath::Vector3(0.f, 0.f, 3.f));
		~Camera() = default;
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;

		// View projection
		LibMath::Matrix4 GetViewMatrix() const;
		LibMath::Matrix4 GetProjectionMatrix(float aspect, float fovDeg, 
											 float near = 0.1f, float far = 100.f) const;
		LibMath::Vector3 GetPosition() const { return m_position; }
		LibMath::Vector3 GetFront() const { return m_front; }

		// Input processing
		void ProcessKeyboard(bool moveForward, bool moveBackward,
							 bool moveLeft, bool moveRight,
							 bool moveUp, bool moveDown,
							 float deltaTime);

		void ProcessMouse(double dx, double dy, float sensitivity = 0.1f);
		void ProcessScroll(double yoffset);

		float m_speed = 2.5f;
		float m_fov = 45.f;

	private:
		LibMath::Vector3 m_position{ 0.f, 0.f, 3.f };
		LibMath::Vector3 m_front{ 0.f, 0.f, -1.f };
		LibMath::Vector3 m_up{ 0.f, 1.f, 0.f };
		LibMath::Vector3 m_right{ 1.f, 0.f, 0.f };

		float m_pitch = 0.f;
		float m_yaw = -90.f;

		void UpdateVectors();
	};
}

#endif
