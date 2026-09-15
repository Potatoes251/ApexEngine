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
		ResourceEntry(std::unique_ptr<IResource> res, bool persistent = false) : m_resource(std::move(res)), m_persistent(persistent) {}
		ResourceEntry(ResourceEntry const&) = delete;
		ResourceEntry& operator=(ResourceEntry const&) = delete;

		std::unique_ptr<IResource>	m_resource = nullptr;
		std::atomic<int>			m_refCount = 0;
		bool						m_persistent = false;
		float						m_timeSinceUnused = 0.f;
	};	
	
	template <typename T>
	concept HandleType = std::derived_from<T, IResource>;

	template<typename HandleType>
	class ResourceHandle
	{
	public:
		ResourceHandle() = default;
		ResourceHandle(ResourceEntry* resourceEntry) : m_resourceEntry(resourceEntry)
		{
			if (m_resourceEntry) m_resourceEntry->m_refCount.fetch_add(1);
		}

		ResourceHandle(ResourceHandle const& other) : m_resourceEntry(other.m_resourceEntry)
		{
			if (m_resourceEntry) m_resourceEntry->m_refCount.fetch_add(1);
		}
		ResourceHandle& operator=(ResourceHandle const& other) 
		{
			if (this != &other)
			{
				if (m_resourceEntry) m_resourceEntry->m_refCount.fetch_sub(1);

				m_resourceEntry = other.m_resourceEntry;
				if (m_resourceEntry) m_resourceEntry->m_refCount.fetch_add(1);
			}
			return *this;
		}

		~ResourceHandle()
		{
			if (!m_resourceEntry)
				return;

			m_resourceEntry->m_refCount.fetch_sub(1);
		}

		bool IsValid() const
		{
			return m_resourceEntry;
		}
		bool IsReady() const
		{
			return	m_resourceEntry &&
				m_resourceEntry->m_resource &&
				m_resourceEntry->m_resource->GetState() == ResourceState::Uploaded;
		}
		bool IsFailed() const
		{
			return	m_resourceEntry &&
				m_resourceEntry->m_resource &&
				m_resourceEntry->m_resource->GetState() == ResourceState::Failed;
		}
		HandleType* operator->() const
		{
			return m_resourceEntry ? (HandleType*)(m_resourceEntry->m_resource.get()) : nullptr;
		}

	private:
		ResourceEntry*	m_resourceEntry = nullptr;
	};
}


#endif
