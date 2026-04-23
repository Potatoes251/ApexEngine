#ifndef SET_SCRIPT_PATH_COMMAND
#define SET_SCRIPT_PATH_COMMAND

#include "ICommand.h"

#include <string>

namespace Apex::Rendering { class Scene; }
namespace Apex::Scripting { class ScriptComponent; }

namespace Apex::CtrlZ
{
	class SetScriptPathCommand : public ICommand
	{
	public:
		SetScriptPathCommand(Rendering::Scene* scene, Scripting::ScriptComponent* script, std::string oldPath, std::string newPath);

		void Execute() override;
		void Undo() override;

	private:
		Rendering::Scene* m_scene;
		size_t				m_objectId;
		size_t				m_compId;

		std::string			m_oldPath;
		std::string			m_newPath;
	};
}

#endif // !SET_SCRIPT_PATH_COMMAND

