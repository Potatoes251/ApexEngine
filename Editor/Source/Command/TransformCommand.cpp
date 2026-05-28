#include "Command/TransformCommand.h"

#include "Object.h"
#include "Application.h"

Apex::CtrlZ::TransformCommand::TransformCommand(Data::Object* target, LibMath::Transform trans, bool global)
	: m_newTransform(trans), m_global(global)
{
	m_targetId = target->GetId();

	if (global)
		m_oldTransform = target->GetGlobalTransform();
	else
		m_oldTransform = target->GetLocalTransform();
}

void Apex::CtrlZ::TransformCommand::Execute()
{
	Data::Object* target = Application::Get()->GetScene()->GetObjectWithId(m_targetId);

	assert(target != nullptr);

	if (m_global)
		target->SetGlobalTransform(m_newTransform);
	else
		target->SetLocalTransform(m_newTransform);
}

void Apex::CtrlZ::TransformCommand::Undo()
{
	Data::Object* target = Application::Get()->GetScene()->GetObjectWithId(m_targetId);

	assert(target != nullptr);

	if (m_global)
		target->SetGlobalTransform(m_oldTransform);
	else
		target->SetLocalTransform(m_oldTransform);
}
