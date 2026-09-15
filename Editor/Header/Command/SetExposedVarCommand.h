#ifndef SET_MEM_VAR
#define SET_MEM_VAR

#include "ICommand.h"
#include "Component.h"
#include "Application.h"
#include "Object.h"

namespace Apex::CtrlZ
{
	template<typename T>
	class SetExposedVarCommand : public ICommand
	{
	public:
		SetExposedVarCommand(ExposedVar var, Data::Object* obj, Component* comp, T oldValue, T newValue)
		{
			m_targetCompId = comp->GetId();
			m_targetId = obj->GetId();
			m_varName = var.m_name;
			m_newValue = newValue;
			m_oldValue = oldValue;
		}
		~SetExposedVarCommand() = default;

		void Execute() override
		{
			Data::Object* obj = Application::Get()->GetScene()->GetObjectWithId(m_targetId);
			if (!obj) return;

			Component* comp = obj->GetComponent(m_targetCompId);
			if (!comp) return;

			for (ExposedVar& var : comp->GetExposedVariables())
			{
				if (var.m_name == m_varName)
				{
					*(T*)var.m_data = m_newValue;
					return;
				}
			}
		}

		void Undo() override
		{
			Data::Object* obj = Application::Get()->GetScene()->GetObjectWithId(m_targetId);
			if (!obj) return;

			Component* comp = obj->GetComponent(m_targetCompId);
			if (!comp) return;

			for (ExposedVar& var : comp->GetExposedVariables())
			{
				if (var.m_name == m_varName)
				{
					*(T*)var.m_data = m_oldValue;
					return;
				}
			}
		}

	private:
		std::string			m_varName;
		size_t				m_targetId;
		size_t				m_targetCompId;
		T	m_newValue;
		T	m_oldValue;
	};
}



#endif // !SET_MEM_VAR

