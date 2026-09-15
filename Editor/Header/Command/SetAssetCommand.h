#ifndef SET_ASSET_COMMAND
#define SET_ASSET_COMMAND


#include "ICommand.h"

#include "ResourceHandle.h"

namespace Apex::CtrlZ
{
	template<typename T>
	class SetAssetCommand : public ICommand
	{
	public:
		SetAssetCommand(Resources::ResourceHandle<T>* target, Resources::ResourceHandle<T> oldValue, Resources::ResourceHandle<T> newValue)
			: m_target(target), m_old(oldValue), m_new(newValue) {}


		void Execute() override
		{
			*m_target = m_new;
		}

		void Undo() override
		{
			*m_target = m_old;
		}

	private:
		Resources::ResourceHandle<T>*	m_target;
		Resources::ResourceHandle<T>	m_old;
		Resources::ResourceHandle<T>	m_new;
	};
}


#endif // !SET_ASSET_COMMAND

