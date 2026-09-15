#ifndef SET_SCRIPT_VAR_COMMAND
#define SET_SCRIPT_VAR_COMMAND

#include "ICommand.h"

#include "Scene.h"
#include "Object.h"
#include "ScriptComponent.h"


namespace Apex::CtrlZ
{
	template<typename T>
	class SetScriptVarCommand : public ICommand
	{
	public:
		SetScriptVarCommand(Rendering::Scene* scene, Scripting::ScriptComponent* script, std::string const& varName, T oldVal, T newVal)
			: m_scene(scene), m_oldVal(oldVal), m_newVal(newVal), m_varName(varName)
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

		void Execute() override
		{
			Data::Object* owner = m_scene->GetObjectWithId(m_objectId);

			Scripting::ScriptComponent* script = dynamic_cast<Scripting::ScriptComponent*>(owner->GetComponent(m_compId));

			if (!script) return;

			script->SetVariable(m_varName, m_newVal);
		}

		void Undo() override
		{
			Data::Object* owner = m_scene->GetObjectWithId(m_objectId);

			Scripting::ScriptComponent* script = dynamic_cast<Scripting::ScriptComponent*>(owner->GetComponent(m_compId));

			if (!script) return;

			script->SetVariable(m_varName, m_oldVal);
		}

	private:
		std::string m_varName;

		Rendering::Scene*	m_scene;

		size_t	m_objectId;
		size_t	m_compId;

		T	m_oldVal;
		T	m_newVal;
	};
}

#endif // !SET_SCRIPT_VAR_COMMAND