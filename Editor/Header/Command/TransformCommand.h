#ifndef TRANSFORM_COMMAND
#define TRANSFORM_COMMAND

#include "ICommand.h"

#include "LibMath/Transform.h"

namespace Apex::Rendering { class Scene; }
namespace Apex::Data { class Object; }

namespace Apex::CtrlZ
{
	class TransformCommand : public ICommand
	{
	public:
		TransformCommand(Rendering::Scene* m_scene, Data::Object* target, LibMath::Transform trans, bool global);

		void Execute() override;
		void Undo() override;
	private:
		LibMath::Transform	m_oldTransform;
		LibMath::Transform	m_newTransform;
		Rendering::Scene*	m_scene;
		size_t				m_targetId;
		bool				m_global;
	};
}

#endif // !TRANSFORM_COMMAND
