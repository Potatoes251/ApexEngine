#ifndef RESOURCE_HANDLE
#define RESOURCE_HANDLE

#include "IResource.h"

#include <memory>
#include <atomic>

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
		std::atomic<int>			m_refCount = 0;
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
		bool IsReady() const;
		bool IsFailed() const;
		IResource* operator->() const;

	private:
		ResourceEntry*	m_resourceEntry = nullptr;
	};
}


#endif
