#include "Command/RemoveCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Physic.h"

using namespace Apex::CtrlZ;

RemoveCompCommand::RemoveCompCommand(Data::Object* target, Component* comp, Physic::PhysicSystem* physic)
{
	m_savedComp = comp->Clone();
	m_target = target;
	m_compId = comp->GetId();
	m_physic = physic;
}

void RemoveCompCommand::Execute()
{
	m_target->RemoveComponent(m_compId, m_physic);
}

void Apex::CtrlZ::RemoveCompCommand::Undo()
{
	std::unique_ptr<Component> clone = m_savedComp->Clone();
	size_t id = m_target->AddComponentDirect(std::move(clone));
	m_target->SetCompId(id, m_compId);
}
