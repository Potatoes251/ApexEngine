#ifndef ADD_COMP_COMMAND
#define ADD_COMP_COMMAND

#include "ICommand.h"

#include <functional>

namespace Apex::Data { class Object; }
namespace Apex { class Component; }

namespace Apex::CtrlZ
{
	using createFunc = std::function<size_t (Apex::Data::Object*)>;


	class AddCompCommand : public ICommand
	{
	public:
		AddCompCommand(Data::Object* target, createFunc createFunc);

		void Execute() override;
		void Undo() override;


	private:
		createFunc m_createFunc;
		
		size_t	m_targetId;
		size_t	m_compId;
	};
}


#endif // !ADD_COMP_COMMAND
