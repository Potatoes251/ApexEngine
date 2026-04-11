#include "Command/TransformCommand.h"

#include "Object.h"

Apex::CtrlZ::TransformCommand::TransformCommand(Data::Object* target, LibMath::Transform old, LibMath::Transform niou, bool global)
	: m_target(target), m_oldTransform(old), m_newTransform(niou), m_global(global)
{}

void Apex::CtrlZ::TransformCommand::Execute()
{
	if (m_global)
		m_target->SetGlobalTransform(m_newTransform);
	else
		m_target->SetLocalTransform(m_newTransform);
}

void Apex::CtrlZ::TransformCommand::Undo()
{
	if (m_global)
		m_target->SetGlobalTransform(m_oldTransform);
	else
		m_target->SetLocalTransform(m_oldTransform);
}
