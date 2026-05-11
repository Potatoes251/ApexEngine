#ifndef VIEW_COMPONENT
#define VIEW_COMPONENT

#include <memory>
#include "Component.h"
#include "Physic.h"
#include "LibMath/Vector/Vector3.h"

namespace Apex::Perception
{
	class ViewComponent : public Component
	{
	public:
		ViewComponent() = delete;
		ViewComponent(Apex::Physic::PhysicSystem* physic, float range, float horizontalFov, float verticalFov, int horizontalRays, int verticalRays);
		~ViewComponent() = default ;
		ViewComponent(const ViewComponent& other);
		ViewComponent& operator=(const ViewComponent& other) = default;

		//Component
		void						Serialize(std::ostream& out) const override;
		std::unique_ptr<Component>	Clone() override;
		const char*					GetTypeName() const override { return "ViewComponent"; }
		std::vector<ExposedVar>		GetExposedVariables() override;

		void OnFixedUpdate(float fixedDeltaTime_s) override;
		void OnStart() override;

		const std::vector<Apex::Physic::HitResult>& GetHits() const { return m_Hits; }
		size_t GetNumberOfHits() const { return m_Hits.size(); }

	private:
		void CastRay(const LibMath::Vector3& origin, const LibMath::Vector3& dir);

		float	m_range = 10.f;
		float	m_horizontalFov = 1.5f;	//in rad
		float	m_verticalFov = 1.f;	//in rad

		int		m_horizontalRays = 10;
		int		m_verticalRays = 10;

		std::vector<LibMath::Vector3>		m_localDirections;

		std::vector<Apex::Physic::HitResult>	m_Hits;

		Apex::Physic::PhysicSystem*		m_physicSystem = nullptr;
	};
}

#endif // !VIEW_COMPONENT
