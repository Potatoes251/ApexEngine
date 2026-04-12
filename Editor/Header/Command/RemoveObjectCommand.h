#ifndef REMOVE_OBJECT_COMMAND
#define REMOVE_OBJECT_COMMAND


#include "ICommand.h"

#include <functional>
#include <memory>

namespace Apex::Data { class Object; }

namespace Apex::CtrlZ
{
	using addObjectFunc = std::function<void (std::unique_ptr<Apex::Data::Object>)>;
	using destroyFunc = std::function<void(size_t)>;


	class RemoveObjectCommand : public ICommand
	{
	public:
		RemoveObjectCommand(Data::Object* target, addObjectFunc addObjectFunc, destroyFunc destroyFunc);

		void Execute() override;
		void Undo() override;


	private:
		addObjectFunc	m_addObjectFunc;
		destroyFunc		m_destroyFunc;
		std::unique_ptr<Data::Object> m_savedObject;
		Data::Object*	m_target;
		size_t			m_targetId;
	};
}


#endif // !REMOVE_OBJECT_COMMAND

