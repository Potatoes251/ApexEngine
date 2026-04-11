#ifndef ADD_COMP_COMMAND
#define ADD_COMP_COMMAND

#include "ICommand.h"

#include <functional>

namespace Apex::Physic { class PhysicSystem; }
namespace Apex::Data { class Object; }
namespace Apex { class Component; }


namespace Apex::CtrlZ
{
	using createFunc = std::function<Apex::Component* (Apex::Data::Object*)>;


	class AddCompCommand : public ICommand
	{
	public:
		AddCompCommand(Data::Object* target, createFunc createFunc);
		AddCompCommand(Data::Object* target, Physic::PhysicSystem* physic, createFunc createFunc);

		void Execute() override;
		void Undo() override;


	private:
		createFunc m_createFunc;
		
		Data::Object* m_target;
		Component* m_comp = nullptr;

		Physic::PhysicSystem* m_physic = nullptr;
	};
}


#endif // !ADD_COMP_COMMAND
