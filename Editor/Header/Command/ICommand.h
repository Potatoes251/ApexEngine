#ifndef COMMAND
#define COMMAND

namespace Apex::CtrlZ
{
	// Interface for all commands affected by ctrlZ
	class ICommand
	{
	public:
		virtual ~ICommand() = default;

		virtual void Execute() = 0;
		virtual void Undo() = 0;

	private:

	};
}


#endif // !COMMAND
