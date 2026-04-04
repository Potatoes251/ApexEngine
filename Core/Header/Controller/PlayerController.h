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


		void Serialize(std::ostream& out) const override;

		std::unique_ptr<Component> Clone() const override { return std::make_unique<PlayerController>(*this); }

		const char* GetTypeName() const override { return "PlayerController"; }
		std::vector<ExposedVar> GetExposedVariables() override { return std::vector<ExposedVar>(); }

	private:

	};
}



#endif // !PLAYER_CONTROLLER