#ifndef REMOVE_COMP_COMMAND
#define REMOVE_COMP_COMMAND

#include "ICommand.h"

#include <memory>

namespace Apex::Data { class Object; }
namespace Apex { class Component; }

namespace Apex::CtrlZ
{
	class RemoveCompCommand : public ICommand
	{
	public:
		RemoveCompCommand(Data::Object* target, Component* comp);

		void Execute() override;
		void Undo() override;


	private:
		std::unique_ptr<Component> m_savedComp = nullptr;
		size_t	m_targetId;
		size_t	m_compId;
	};
}

#endif // !REMOVE_COMP_COMMAND
