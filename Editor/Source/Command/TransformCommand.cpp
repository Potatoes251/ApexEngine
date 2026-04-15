#include "Command/TransformCommand.h"

#include "Object.h"
#include "Scene.h"

Apex::CtrlZ::TransformCommand::TransformCommand(Rendering::Scene* scene, Data::Object* target, LibMath::Transform trans, bool global)
	: m_newTransform(trans), m_global(global)
{
	m_scene = scene;

	m_targetId = target->GetId();

	if (global)
		m_oldTransform = target->GetGlobalTransform();
	else
		m_oldTransform = target->GetLocalTransform();
}

void Apex::CtrlZ::TransformCommand::Execute()
{
	Data::Object* target = m_scene->GetObjectWithId(m_targetId);
	if (!target) return;

	if (m_global)
		target->SetGlobalTransform(m_newTransform);
	else
		target->SetLocalTransform(m_newTransform);
}

void Apex::CtrlZ::TransformCommand::Undo()
{
	Data::Object* target = m_scene->GetObjectWithId(m_targetId);
	if (!target) return;

	if (m_global)
		target->SetGlobalTransform(m_oldTransform);
	else
		target->SetLocalTransform(m_oldTransform);
}
