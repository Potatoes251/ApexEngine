#include "CommandManager.h"

#include "Log.h"

using namespace Apex::CtrlZ;

CommandManager& CommandManager::Get()
{
	static CommandManager instance;
	return instance;
}

void CommandManager::Execute(std::unique_ptr<ICommand> cmd)
{
	if (!cmd) return;

	cmd->Execute();
	if (!m_batching)
	{
		m_undoStack.push_back(std::move(cmd));

		if (m_undoStack.size() > MAX_SIZE)
		{
			m_undoStack.pop_front();
		}
	}
	else
	{
		m_batch->AddCommand(std::move(cmd));
	}

	// empty
	m_redoStack.clear();
}

void CommandManager::StartBatch()
{
	if (m_batching)
	{
		return;
	}

	m_batching = true;
	m_batch = std::make_unique<BatchCommand>();
}

void CommandManager::EndBatch()
{
	if (!m_batching)
	{
		return;
	}

	m_batching = false;
	if (m_batch && !m_batch->IsEmpty())
	{
		m_undoStack.push_back(std::move(m_batch));

		if (m_undoStack.size() > MAX_SIZE)
		{
			m_undoStack.pop_front();
		}

		m_redoStack.clear();
	}
}

void CommandManager::Undo()
{
	if (m_undoStack.empty()) return;

	std::unique_ptr<ICommand> cmd = std::move(m_undoStack.back());
	m_undoStack.pop_back();

	cmd->Undo();

	m_redoStack.push_back(std::move(cmd));
}

void CommandManager::Redo()
{
	if (m_redoStack.empty()) return;

	std::unique_ptr<ICommand> cmd = std::move(m_redoStack.back());
	m_redoStack.pop_back();

	cmd->Execute();

	m_undoStack.push_back(std::move(cmd));
}