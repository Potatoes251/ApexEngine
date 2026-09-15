#include "Command/AddObjectCommand.h"

#include "Object.h"

using namespace Apex::CtrlZ;


AddObjectCommand::AddObjectCommand(createFunc createFunc, destroyFunc destroyFunc)
{
	m_createFunc = createFunc;
	m_destroyFunc = destroyFunc;
}

void AddObjectCommand::Execute()
{
	m_target = m_createFunc();
	m_targetId = m_target->GetId();
}

void AddObjectCommand::Undo()
{
	m_destroyFunc(m_targetId);
	m_target = nullptr;
}
