#ifndef TRANSFORM_COMMAND
#define TRANSFORM_COMMAND

#include "ICommand.h"

#include "LibMath/Transform.h"

namespace Apex::Data { class Object; }

namespace Apex::CtrlZ
{
	class TransformCommand : public ICommand
	{
	public:
		TransformCommand() = delete;
		TransformCommand(Data::Object* target, LibMath::Transform old, LibMath::Transform niou, bool global);

		void Execute() override;
		void Undo() override;
	private:
		LibMath::Transform	m_oldTransform;
		LibMath::Transform	m_newTransform;
		Data::Object*		m_target;
		bool				m_global;
	};
}

#endif // !TRANSFORM_COMMAND
