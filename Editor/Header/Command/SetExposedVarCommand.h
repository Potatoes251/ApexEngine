#ifndef SET_MEM_VAR
#define SET_MEM_VAR

#include "ICommand.h"
#include "Component.h"
#include "Scene.h"
#include "Object.h"

namespace Apex::CtrlZ
{
	template<typename T>
	class SetExposedVarCommand : public ICommand
	{
	public:
		SetExposedVarCommand(ExposedVar var, Data::Object* obj, Component* comp, T oldValue, T newValue, Rendering::Scene* scene)
			: m_scene(scene)
		{
			m_targetCompId = comp->GetId();
			m_targetId = obj->GetId();
			m_varName = var.name;
			m_newValue = newValue;
			m_oldValue = oldValue;
		}
		~SetExposedVarCommand() = default;

		void Execute() override
		{
			Object* obj = m_scene->GetObjectWithId(m_targetId);
			if (!obj) return;

			Component* comp = obj->GetComponent(m_targetCompId);
			if (!comp) return;

			for (ExposedVar& var : comp->GetExposedVariables())
			{
				if (var.name == m_varName)
				{
					*(T*)var.data = m_newValue;
					return;
				}
			}
		}

		void Undo() override
		{
			Object* obj = m_scene->GetObjectWithId(m_targetId);
			if (!obj) return;

			Component* comp = obj->GetComponent(m_targetCompId);
			if (!comp) return;

			for (ExposedVar& var : comp->GetExposedVariables())
			{
				if (var.name == m_varName)
				{
					*(T*)var.data = m_oldValue;
					return;
				}
			}
		}

	private:
		std::string			m_varName;
		size_t				m_targetId;
		size_t				m_targetCompId;
		Rendering::Scene*	m_scene;
		T	m_newValue;
		T	m_oldValue;
	};
}



#endif // !SET_MEM_VAR

