#ifndef ADD_OBJECT_COMMAND
#define ADD_OBJECT_COMMAND


#include "ICommand.h"

#include <functional>

namespace Apex::Data { class Object; }

namespace Apex::CtrlZ
{
	using createFunc = std::function<Apex::Data::Object* (void)>;
	using destroyFunc = std::function<void (Apex::Data::Object*)>;


	class AddObjectCommand : public ICommand
	{
	public:
		AddObjectCommand(createFunc createFunc, destroyFunc destroyFunc);

		void Execute() override;
		void Undo() override;


	private:
		Data::Object*	m_target;
		createFunc		m_createFunc;
		destroyFunc		m_destroyFunc;
	};
}

#endif // !ADD_OBJECT_COMMAND

