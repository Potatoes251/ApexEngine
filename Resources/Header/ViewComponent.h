#ifndef VIEW_COMPONENT
#define VIEW_COMPONENT

#include <memory>
#include "../../Resources/header/Component.h"
#include "../../Physic/Header/Physic.h"
#include "../../Physic/Header/Rigidbody.h"
#include "../../Resources/header/Object.h"
#include "LibMath/Vector/Vector3.h"
#include "LibMath/Angle/Degree.h"
#include "LibMath/Angle/Radian.h"

using std::unique_ptr;
using std::vector;
using Apex::Physic::PhysicSystem;
using Apex::Physic::HitResult;
using LibMath::Vector3;

namespace Apex::Perception
{
	class ViewComponent : public Component
	{
	public:
		ViewComponent() = delete;
		ViewComponent(PhysicSystem* physic, float range, float horizontalFov, float verticalFov, int horizontalRays, int verticalRays);
		~ViewComponent() = default ;
		ViewComponent(const ViewComponent& other);
		ViewComponent& operator=(const ViewComponent& other) = default;

		//Component
		void					Serialize(std::ostream& out) const override;
		unique_ptr<Component>	Clone() override;
		const char*				GetTypeName() const override		{ return "ViewComponent"; };
		vector<ExposedVar>		GetExposedVariables() override;

		void OnFixedUpdate(float fixedDeltaTime_s) override;
		void OnStart() override;

		const vector<HitResult>& GetHits()	{ return m_Hits; }
		int GetNuberOfHits()				{ return m_Hits.size(); }

	private:
		void castRay(const Vector3& origin, const Vector3& dir);

		float	m_range = 10.f;
		float	m_horizontalFov = 1.5f;	//in rad
		float	m_verticalFov = 1.f;	//in rad

		int		m_horizontalRays = 10;
		int		m_verticalRays = 10;

		vector<Vector3>		m_localDirections;

		vector<HitResult>	m_Hits;

		PhysicSystem*		m_physicSystem = nullptr;
	};
}

#endif // !VIEW_COMPONENT
