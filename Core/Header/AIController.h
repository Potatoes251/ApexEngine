#ifndef AI_CONTROLLER
#define AI_CONTROLLER

#include "CharacterController.h"

namespace Apex::Pathfinding { class WaypointGraph; }

namespace Apex::Controller
{
	class AIController : public CharacterController
	{
	public:
		AIController() = default;

		void SetArrivalThreshold(float newVal) { m_arrivalThreshold = newVal; }

		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() override { return std::make_unique<AIController>(*this); }

		const char* GetTypeName() const override { return "AIController"; }

		std::vector<ExposedVar> GetExposedVariables() override;

		void SetTarget(Data::Object* newTarget);

		void OnStart() override;
		void OnFixedUpdate(float deltatime_s) override;

	private:
		void SetPath(std::vector<LibMath::Vector3> const& path);

		Data::Object* m_target = nullptr;
		Pathfinding::WaypointGraph* m_graph = nullptr;

		std::vector<LibMath::Vector3> m_path;
		size_t m_pathIdx = 0;

		float m_arrivalThreshold = .5f;
		float m_pathRefreshTimer = 0.f;
		// refresh path each X second
		float m_pathRefreshInterval = .5f;
	};
}

#endif // !AI_CONTROLLER

