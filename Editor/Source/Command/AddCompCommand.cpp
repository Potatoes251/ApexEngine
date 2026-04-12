#include "Command/AddCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Physic.h"

using namespace Apex::CtrlZ;

AddCompCommand::AddCompCommand(Data::Object* target, createFunc createFunc)
{
	m_target = target;
	m_createFunc = createFunc;
}

AddCompCommand::AddCompCommand(Data::Object* target, Physic::PhysicSystem* physic, createFunc createFunc)
{
	m_target = target;
	m_createFunc = createFunc;
	m_physic = physic;
}

void AddCompCommand::Execute()
{
	if (m_target)
	{
		m_compId = m_createFunc(m_target);
	}
}

void AddCompCommand::Undo()
{
	m_target->RemoveComponent(m_compId, m_physic);
}
