#include "Command/AddObjectCommand.h"


using namespace Apex::CtrlZ;


AddObjectCommand::AddObjectCommand(createFunc createFunc, destroyFunc destroyFunc)
{
	m_createFunc = createFunc;
	m_destroyFunc = destroyFunc;
}

void AddObjectCommand::Execute()
{
	m_target = m_createFunc();
}

void AddObjectCommand::Undo()
{
	m_destroyFunc(m_target);
	m_target = nullptr;
}
