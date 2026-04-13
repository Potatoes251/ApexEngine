#ifndef ADD_COMP_COMMAND
#define ADD_COMP_COMMAND

#include "ICommand.h"

#include <functional>

namespace Apex::Physic { class PhysicSystem; }
namespace Apex::Rendering { class Scene; }
namespace Apex::Data { class Object; }
namespace Apex { class Component; }

namespace Apex::CtrlZ
{
	using createFunc = std::function<size_t (Apex::Data::Object*)>;


	class AddCompCommand : public ICommand
	{
	public:
		AddCompCommand(Rendering::Scene* scene, Data::Object* target, createFunc createFunc);
		AddCompCommand(Rendering::Scene* scene, Data::Object* target, Physic::PhysicSystem* physic, createFunc createFunc);

		void Execute() override;
		void Undo() override;


	private:
		createFunc m_createFunc;
		
		size_t	m_targetId;
		size_t	m_compId;

		Rendering::Scene*		m_scene;
		Physic::PhysicSystem*	m_physic = nullptr;
	};
}


#endif // !ADD_COMP_COMMAND
