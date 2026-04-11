#include "CommandManager.h"

#include <cassert>

using namespace Apex::CtrlZ;

void CommandManager::Execute(std::unique_ptr<ICommand> cmd)
{
	if (!cmd) return;

	cmd->Execute();
	if (!m_batching)
		m_undoStack.push(std::move(cmd));
	else
		m_batch->AddCommand(std::move(cmd));

	// empty
	m_redoStack = {};
}

void Apex::CtrlZ::CommandManager::StartBatch()
{
	if (m_batching)
	{
		printf("Warning: StartBatch called twice without EndBatch\n");
		return;
	}

	m_batching = true;
	m_batch = std::make_unique<BatchCommand>();
}

void Apex::CtrlZ::CommandManager::EndBatch()
{
	m_batching = false;
	if (m_batch && !m_batch->IsEmpty())
	{
		m_undoStack.push(std::move(m_batch));
	}
}

void CommandManager::Undo()
{
	if (m_undoStack.empty()) return;

	std::unique_ptr<ICommand> cmd = std::move(m_undoStack.top());
	m_undoStack.pop();

	cmd->Undo();

	m_redoStack.push(std::move(cmd));
}

void CommandManager::Redo()
{
	if (m_redoStack.empty()) return;

	std::unique_ptr<ICommand> cmd = std::move(m_redoStack.top());
	m_redoStack.pop();

	cmd->Execute();

	m_undoStack.push(std::move(cmd));
}

CommandManager& CommandManager::Get()
{
	static CommandManager instance;
	return instance;
}