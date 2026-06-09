#ifndef CAMERA
#define CAMERA

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Arithmetic.h"

#include "../../Resources/Header/Component.h"

#include <array>

namespace Apex::Rendering
{
	class Camera : public Component
	{
	public:
		Camera(LibMath::Vector3 position = LibMath::Vector3(0.f, 0.f, 10.f));
		~Camera() = default;
		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;

		bool	IsMainCamera() const{ return m_isMain; }
		void	SetMainCamera(bool enabled) { m_isMain = enabled; }
		void	SetProjectionMatrix(float aspect = 16.f / 9.f, float near = 0.1f, float far = 200.f);

		float	GetSpeed() const { return m_speed; }
		void	SetSpeed(float speed) { m_speed = LibMath::clamp(speed, 0.5f, m_maxSpeed); }

		void	SetYaw(float degree);
		void	SetPitch(float degree);

		LibMath::Matrix4	GetViewProj();
		LibMath::Matrix4	GetProjection();
		LibMath::Matrix4	GetViewMatrix();
		LibMath::Vector3	GetPosition() const { return m_position; }

		virtual LibMath::Vector3	GetFront() const = 0;
		virtual LibMath::Vector3	GetRight() const = 0;
		virtual LibMath::Vector3	GetUp() const = 0;

		LibMath::Degree				GetPitch() const { return m_pitch; }
		LibMath::Degree				GetYaw()   const { return m_yaw; }
		float	GetNear() const { return m_near; }
		float	GetFar() const { return m_far; }

		std::array<LibMath::Vector3, 8> GetFrustrumCorners();

		void SetPosition(LibMath::Vector3 pos);
		virtual void Teleport(LibMath::Vector3 pos) { SetPosition(pos); }

		virtual void OnStart() override;

		virtual std::vector<ExposedVar> GetExposedVariables() override;

		virtual void ProcessMouse(double dx, double dy, float sensitivity = 0.1f) = 0;

	protected:
		virtual void UpdateMatrices() = 0;

		LibMath::Matrix4 m_viewProj;
		LibMath::Matrix4 m_proj;
		LibMath::Matrix4 m_view;
		LibMath::Vector3 m_position;

		LibMath::Degree m_fov = LibMath::Degree(45.f);

		LibMath::Degree m_pitch = LibMath::Degree(0.f);
		LibMath::Degree m_yaw = LibMath::Degree(-90.f);

		float			m_speed = 2.5f;
		float			m_maxSpeed = 100.f;

		float			m_near = 0.1f;
		float			m_far = 100.f;

		bool			m_dirty = true;
		bool			m_isMain = false;
	};
}

#endif
