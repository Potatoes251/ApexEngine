#include "Command/SetScriptPathCommand.h"


#include "ScriptComponent.h"
#include "Object.h"
#include "Application.h"

using namespace Apex::CtrlZ;

SetScriptPathCommand::SetScriptPathCommand(Scripting::ScriptComponent* script, std::string oldPath, std::string newPath)
	:m_oldPath(oldPath), m_newPath(newPath)
{
	if (script)
	{
		m_compId = script->GetId();

		Data::Object* owner = script->GetOwner();

		if (owner)
		{
			m_objectId = owner->GetId();
		}
	}
}

void SetScriptPathCommand::Execute()
{
	Data::Object* owner = Application::Get()->GetScene()->GetObjectWithId(m_objectId);

	Scripting::ScriptComponent* script = dynamic_cast<Scripting::ScriptComponent*>(owner->GetComponent(m_compId));

	assert(script != nullptr);

	script->SetScript(m_newPath);
}

void SetScriptPathCommand::Undo()
{
	Data::Object* owner = Application::Get()->GetScene()->GetObjectWithId(m_objectId);

	Scripting::ScriptComponent* script = dynamic_cast<Scripting::ScriptComponent*>(owner->GetComponent(m_compId));

	assert(script != nullptr);

	script->SetScript(m_oldPath);
}
