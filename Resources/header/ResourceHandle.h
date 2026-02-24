#ifndef RESOURCE_HANDLE
#define RESOURCE_HANDLE

#include <memory>
#include <shared_mutex>

#include "IResource.h"

namespace Apex::Resources
{
	struct ResourceEntry
	{
		ResourceEntry() = default;
		ResourceEntry(std::unique_ptr<IResource> res, bool persistent) : m_resource(std::move(res)), m_persistent(persistent)
		{}
		ResourceEntry(ResourceEntry const&) = delete;
		ResourceEntry& operator=(ResourceEntry const&) = delete;

		std::unique_ptr<IResource>	m_resource = nullptr;
		int							m_refCount = 0;
		bool						m_persistent = false;
	};	
	
	
	class ResourceHandle
	{
	public:
		ResourceHandle() = default;
		ResourceHandle(ResourceEntry* resourceEntry);

		ResourceHandle(ResourceHandle const& other);
		ResourceHandle& operator=(ResourceHandle const& other);

		~ResourceHandle();

		bool IsValid() const;
		IResource* operator->() const;

	private:
		ResourceEntry*	m_resourceEntry = nullptr;
	};
}


#endif
