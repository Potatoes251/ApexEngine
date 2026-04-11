#ifndef REMOVE_COMP_COMMAND
#define REMOVE_COMP_COMMAND

#include "ICommand.h"

#include <memory>

namespace Apex::Physic { class PhysicSystem; }
namespace Apex::Data { class Object; }
namespace Apex { class Component; }

namespace Apex::CtrlZ
{
	class RemoveCompCommand : public ICommand
	{
	public:
		RemoveCompCommand(Data::Object* target, Component* m_comp, Physic::PhysicSystem* physic = nullptr);

		void Execute() override;
		void Undo() override;


	private:
		Data::Object* m_target;
		Component* m_comp = nullptr;
		std::unique_ptr<Component> m_savedComp = nullptr;

		Physic::PhysicSystem* m_physic = nullptr;
	};
}

#endif // !REMOVE_COMP_COMMAND
