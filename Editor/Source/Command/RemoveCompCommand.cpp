#include "Command/RemoveCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Physic.h"
#include "Application.h"

using namespace Apex::CtrlZ;

RemoveCompCommand::RemoveCompCommand(Data::Object* target, Component* comp)
{
	m_savedComp = comp->Clone();
	m_targetId = target->GetId();
	m_compId = comp->GetId();
}

void RemoveCompCommand::Execute()
{
	Rendering::Scene* scene = Application::Get()->GetScene();
	Data::Object* target = scene->GetObjectWithId(m_targetId);
	target->RemoveComponent(m_compId, scene->GetPhysic());
}

void RemoveCompCommand::Undo()
{
	Data::Object* target = Application::Get()->GetScene()->GetObjectWithId(m_targetId);
	std::unique_ptr<Component> clone = m_savedComp->Clone();
	m_compId = target->AddComponentDirect(std::move(clone));
}
