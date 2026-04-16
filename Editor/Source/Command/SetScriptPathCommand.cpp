#include "Command/SetScriptPathCommand.h"


#include "ScriptComponent.h"
#include "Object.h"
#include "Scene.h"

using namespace Apex::CtrlZ;

SetScriptPathCommand::SetScriptPathCommand(Rendering::Scene* scene, Scripting::ScriptComponent* script, std::string oldPath, std::string newPath)
	:m_scene(scene), m_oldPath(oldPath), m_newPath(newPath)
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
	Data::Object* owner = m_scene->GetObjectWithId(m_objectId);

	Scripting::ScriptComponent* script = dynamic_cast<Scripting::ScriptComponent*>(owner->GetComponent(m_compId));

	if (!script) return;

	script->SetScript(m_newPath);
}

void SetScriptPathCommand::Undo()
{
	Data::Object* owner = m_scene->GetObjectWithId(m_objectId);

	Scripting::ScriptComponent* script = dynamic_cast<Scripting::ScriptComponent*>(owner->GetComponent(m_compId));

	if (!script) return;

	script->SetScript(m_oldPath);
}
