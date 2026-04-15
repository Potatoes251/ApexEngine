#include "Command/AddCompCommand.h"

#include "Object.h"
#include "Component.h"
#include "Physic.h"
#include "Scene.h"

using namespace Apex::CtrlZ;

AddCompCommand::AddCompCommand(Rendering::Scene* scene, Data::Object* target, createFunc createFunc)
{
	m_scene = scene;
	m_targetId = target->GetId();
	m_createFunc = createFunc;
}

AddCompCommand::AddCompCommand(Rendering::Scene* scene, Data::Object* target, Physic::PhysicSystem* physic, createFunc createFunc)
{
	m_scene = scene;
	m_targetId = target->GetId();
	m_createFunc = createFunc;
	m_physic = physic;
}

void AddCompCommand::Execute()
{
	Data::Object* target = m_scene->GetObjectWithId(m_targetId);
	if (target)
	{
		m_compId = m_createFunc(target);
	}
}

void AddCompCommand::Undo()
{
	Data::Object* target = m_scene->GetObjectWithId(m_targetId);
	if (target)
	{
		target->RemoveComponent(m_compId, m_physic);
	}
}
