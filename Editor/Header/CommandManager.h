#ifndef COMMAND_MANAGER
#define COMMAND_MANAGER


#include "Command/ICommand.h"
#include "Command/BatchCommand.h"

#include <deque>
#include <memory>


namespace Apex::CtrlZ
{
	// Handle Ctrlz
	// Only one Instance of it
	class CommandManager
	{
	public:
		CommandManager(CommandManager const&) = delete;
		CommandManager& operator=(CommandManager const&) = delete;

		static CommandManager& Get();

		void Execute(std::unique_ptr<ICommand> cmd);

		void StartBatch();
		void EndBatch();

		void Undo();
		void Redo();

	private:
		CommandManager() = default;

		static inline constexpr size_t MAX_SIZE = 100;

		// for ctrl-Z
		std::deque<std::unique_ptr<ICommand>> m_undoStack;
		// for ctrl-Y
		std::deque<std::unique_ptr<ICommand>> m_redoStack;

		std::unique_ptr<BatchCommand> m_batch;

		bool m_batching = false;
	};
}



#endif // !COMMAND_MANAGER

