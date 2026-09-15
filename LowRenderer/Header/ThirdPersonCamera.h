#ifndef THIRD_PERSON_CAMERA
#define THIRD_PERSON_CAMERA

#include "Camera.h"

namespace Apex::Rendering
{
	class ThirdPersonCamera : public Camera
	{
	public:
		ThirdPersonCamera() = default;
		ThirdPersonCamera(LibMath::Vector3 target, float distance);
		ThirdPersonCamera(ThirdPersonCamera const& other);
		ThirdPersonCamera& operator=(ThirdPersonCamera const& other);
		~ThirdPersonCamera() = default;

		void ProcessMouse(double dx, double dy, float sensitivity = 0.1f) override;

		void SetDistance(float distance) { m_distance = distance; }
		void SetSpeed(float speed) { m_speed = speed; }

		LibMath::Vector3 GetTarget() const { return m_target; }
		LibMath::Vector3 GetFront() const override { return (m_target - m_position).normalized(); }
		LibMath::Vector3 GetRight() const override { return GetFront().cross(LibMath::Vector3::up()).normalized(); }
		LibMath::Vector3 GetUp() const override { return GetRight().cross(GetFront()).normalized(); }

		const char* GetTypeName() const override { return "ThirdPersonCamera"; }
		void Serialize(std::ostream& out) const override;
		std::vector<ExposedVar> GetExposedVariables() override;

		std::unique_ptr<Component> Clone() override { return std::make_unique<ThirdPersonCamera>(*this); }

		void Teleport(LibMath::Vector3 pos) override;
		void OnStart() override;
		void OnLateUpdate(float deltatime_s) override;

	private:
		void UpdateMatrices() override;

		LibMath::Vector3 m_target;
		float m_distance = 10.f;
	};
}

#endif // !FIRST_PERSON_CAMERA

