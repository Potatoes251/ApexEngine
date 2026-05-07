#include "Command/AddCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Application.h"

using namespace Apex::CtrlZ;

AddCompCommand::AddCompCommand(Data::Object* target, createFunc createFunc)
{
	m_targetId = target->GetId();
	m_createFunc = createFunc;
}

void AddCompCommand::Execute()
{
	Data::Object* target = Application::Get()->GetScene()->GetObjectWithId(m_targetId);
	if (target)
	{
		m_compId = m_createFunc(target);
	}
}

void AddCompCommand::Undo()
{
	Rendering::Scene* scene = Application::Get()->GetScene();
	Data::Object* target = scene->GetObjectWithId(m_targetId);
	if (target)
	{
		target->RemoveComponent(m_compId, scene->GetPhysic());
	}
}
