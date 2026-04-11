#include "Command/RemoveCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Physic.h"

using namespace Apex::CtrlZ;

RemoveCompCommand::RemoveCompCommand(Data::Object* target, Component* comp, Physic::PhysicSystem* physic)
{
	m_target = target;
	m_comp = comp;
	m_physic = physic;
}

void RemoveCompCommand::Execute()
{
	m_savedComp = m_comp->Clone();
	m_target->RemoveComponent(m_comp, m_physic);
}

void Apex::CtrlZ::RemoveCompCommand::Undo()
{
	m_comp = m_savedComp.get();
	m_target->AddComponentDirect(std::move(m_savedComp));
}
