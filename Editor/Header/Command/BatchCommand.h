#ifndef BATCH_COMMAND
#define BATCH_COMMAND

#include "ICommand.h"

#include <vector>
#include <memory>

namespace Apex::CtrlZ
{
	class BatchCommand : public ICommand
	{
	public:
		BatchCommand() = default;

		void AddCommand(std::unique_ptr<ICommand> cmd);
		bool IsEmpty() { return m_commands.empty(); }

		void Execute() override;
		void Undo() override;

	private:
		std::vector<std::unique_ptr<ICommand>> m_commands;
	};
}

#endif // !BATCH_COMMAND
