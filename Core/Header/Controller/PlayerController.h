#ifndef PLAYER_CONTROLLER
#define PLAYER_CONTROLLER

#include "Controller/CharacterController.h"


namespace Apex::Controller
{
	class PlayerController : public CharacterController
	{
	public:
		PlayerController() {}
		PlayerController(PlayerController const& other) {}
		PlayerController& operator=(PlayerController const& other) { return *this; }
		~PlayerController() = default;


		virtual void Serialize(std::ostream& out) const {}

		virtual std::unique_ptr<Component> Clone() const { return std::make_unique<PlayerController>(*this); }

		virtual const char* GetTypeName() const { return "PlayerController"; }
		virtual std::vector<ExposedVar> GetExposedVariables() { return std::vector<ExposedVar>(); }

	private:

	};
}



#endif // !PLAYER_CONTROLLER