#include "Command/RemoveCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Physic.h"
#include "Scene.h"

using namespace Apex::CtrlZ;

RemoveCompCommand::RemoveCompCommand(Rendering::Scene* scene, Data::Object* target, Component* comp, Physic::PhysicSystem* physic)
{
	m_scene = scene;
	m_savedComp = comp->Clone();
	m_targetId = target->GetId();
	m_compId = comp->GetId();
	m_physic = physic;
}

void RemoveCompCommand::Execute()
{
	Data::Object* target = m_scene->GetObjectWithId(m_targetId);
	target->RemoveComponent(m_compId, m_physic);
}

void RemoveCompCommand::Undo()
{
	Data::Object* target = m_scene->GetObjectWithId(m_targetId);
	std::unique_ptr<Component> clone = m_savedComp->Clone();
	size_t id = target->AddComponentDirect(std::move(clone));
	target->SetCompId(id, m_compId);
}
