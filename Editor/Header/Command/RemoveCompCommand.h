#ifndef REMOVE_COMP_COMMAND
#define REMOVE_COMP_COMMAND

#include "ICommand.h"

#include <memory>

namespace Apex::Physic { class PhysicSystem; }
namespace Apex::Rendering { class Scene; }
namespace Apex::Data { class Object; }
namespace Apex { class Component; }

namespace Apex::CtrlZ
{
	class RemoveCompCommand : public ICommand
	{
	public:
		RemoveCompCommand(Rendering::Scene* scene, Data::Object* target, Component* comp, Physic::PhysicSystem* physic = nullptr);

		void Execute() override;
		void Undo() override;


	private:
		std::unique_ptr<Component> m_savedComp = nullptr;
		size_t	m_targetId;
		size_t	m_compId;

		Rendering::Scene*	  m_scene;
		Physic::PhysicSystem* m_physic = nullptr;
	};
}

#endif // !REMOVE_COMP_COMMAND
