#include "Command/RemoveObjectCommand.h"

#include "Object.h"

using namespace Apex::CtrlZ;


RemoveObjectCommand::RemoveObjectCommand(Data::Object* target, addObjectFunc addObjectFunc, destroyFunc destroyFunc)
{
	m_target = target;
	m_addObjectFunc = addObjectFunc;
	m_destroyFunc = destroyFunc;
}

void RemoveObjectCommand::Execute()
{
	m_savedObject = m_target->Clone();
	m_destroyFunc(m_target);
	m_target = nullptr;
}

void RemoveObjectCommand::Undo()
{
	m_addObjectFunc(std::move(m_savedObject));
}
