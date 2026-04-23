#include "Command/RemoveObjectCommand.h"

#include "Object.h"

using namespace Apex::CtrlZ;


RemoveObjectCommand::RemoveObjectCommand(Data::Object* target, addObjectFunc addObjectFunc, destroyFunc destroyFunc)
{
	m_targetId = target->GetId();
	m_savedObject = target->Clone();
	m_savedObject->RemoveId();
	m_addObjectFunc = addObjectFunc;
	m_destroyFunc = destroyFunc;
}

void RemoveObjectCommand::Execute()
{
	m_destroyFunc(m_targetId);
}

void RemoveObjectCommand::Undo()
{
	std::unique_ptr<Data::Object> newObj = m_savedObject->Clone();
	newObj->SetId(m_targetId);
	m_addObjectFunc(std::move(newObj));
}
