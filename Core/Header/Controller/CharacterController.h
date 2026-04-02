#ifndef CHARACTER_CONTROLLER
#define CHARACTER_CONTROLLER

#include "Component.h"

namespace Apex::Controller
{
	class CharacterController : public Component
	{
	public:
		CharacterController() = default;
		CharacterController(CharacterController const& other) = default;
		CharacterController& operator=(CharacterController const& other) = default;
		~CharacterController() = default;

	protected:



	private:


	};
}


#endif // !CHARACTER_CONTROLLER
