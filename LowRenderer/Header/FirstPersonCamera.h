#ifndef FIRST_PERSON_CAMERA
#define FIRST_PERSON_CAMERA

#include "Camera.h"

namespace Apex::Rendering
{
	class FirstPersonCamera : public Camera
	{
	public:
		FirstPersonCamera(LibMath::Vector3 position = LibMath::Vector3(0.f, 0.f, 10.f));
		FirstPersonCamera(FirstPersonCamera const& other);
		FirstPersonCamera& operator=(FirstPersonCamera const& other);

		void ProcessKeyboard(bool moveForward, bool moveBackward,
			bool moveLeft, bool moveRight,
			bool moveUp, bool moveDown,
			float deltaTime);

		LibMath::Vector3 GetFront() const override { return m_front; }
		LibMath::Vector3 GetRight() const override { return m_right; }
		LibMath::Vector3 GetUp() const override { return m_up; }

		void ProcessMouse(double dx, double dy, float sensitivity = 0.1f) override;
		void ProcessScroll(double yoffset);

		const char* GetTypeName() const override { return "FirstPersonCamera"; }
		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() override { return std::make_unique<FirstPersonCamera>(*this); }

		void OnLateUpdate(float /*deltatime_s*/) override;

		void SetOrientation(LibMath::Degree pitch, LibMath::Degree yaw);

	private:
		void UpdateVectors();
		void UpdateMatrices() override;

		LibMath::Vector3 m_front{ 0.f, 0.f, -1.f };
		LibMath::Vector3 m_up{ 0.f, 1.f, 0.f };
		LibMath::Vector3 m_right{ 1.f, 0.f, 0.f };
	};
}

#endif // !FIRST_PERSON_CAMERA
