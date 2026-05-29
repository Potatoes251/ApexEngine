#ifndef SET_VAL_COMMAND
#define SET_VAL_COMMAND

#include "ICommand.h"


namespace Apex::CtrlZ
{
	template<typename T>
	class SetValCommand : public ICommand
	{
	public:
		SetValCommand(T* valPtr, T oldVal, T newVal) 
			: m_valPtr(valPtr), m_oldVal(oldVal), m_newVal(newVal)
		{}

		void Execute() override
		{
			*m_valPtr = m_newVal;
		}

		void Undo() override
		{
			*m_valPtr = m_oldVal;
		}

	private:
		T*	m_valPtr;
		T	m_oldVal;
		T	m_newVal;
	};
}


#endif // !SET_VAL_COMMAND

