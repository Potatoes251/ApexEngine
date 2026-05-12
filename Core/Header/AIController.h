#ifndef AI_CONTROLLER
#define AI_CONTROLLER

#include "CharacterController.h"

namespace Apex::Controller
{
	class AIController : public CharacterController
	{
	public:
		AIController() = default;

		std::unique_ptr<Component> Clone() override { return std::make_unique<AIController>(*this); }

		const char* GetTypeName() const override { return "AIController"; }

		void SetTarget(Data::Object* newTarget) { m_target = newTarget; }

		void OnFixedUpdate(float deltatime_s) override;

	private:
		Data::Object* m_target = nullptr;

		float m_arrivalThreshold = .1f;
	};
}

#endif // !AI_CONTROLLER

