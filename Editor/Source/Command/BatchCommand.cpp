#include "Command/BatchCommand.h"



void Apex::CtrlZ::BatchCommand::AddCommand(std::unique_ptr<ICommand> cmd)
{
	m_commands.push_back(std::move(cmd));
}

void Apex::CtrlZ::BatchCommand::Execute()
{
	for (auto& cmd : m_commands)
		cmd->Execute();
}

void Apex::CtrlZ::BatchCommand::Undo()
{
	for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it)
		(*it)->Undo();
}
