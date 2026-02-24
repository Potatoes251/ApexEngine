#ifndef RESOURCE_MANAGER
#define RESOURCE_MANAGER

#include "ResourceHandle.h"
#include "ThreadPool.h"
 
#include <unordered_map>
#include <string>
#include <shared_mutex>


namespace Apex::Resources
{
	// restrict template type for create and get function
	template <typename T>
	concept ResourceType = std::derived_from<T, IResource> && std::constructible_from<T, std::string const&>;


	class ResourceManager
	{
	public:
		ResourceManager(Core::ThreadPool& threadPool) : m_threadPool(threadPool) {}
		ResourceManager(ResourceManager const&) = delete;
		ResourceManager& operator=(ResourceManager const&) = delete;
		~ResourceManager() = default;

		template <typename ResourceType>
		ResourceHandle CreateAsync(std::string const& id, bool persistent = false);

		template <typename ResourceType>
		ResourceHandle Get(std::string id);

	private:
		template <typename ResourceType>
		void Create(std::string const& id, bool persistent);

		Core::ThreadPool&	m_threadPool;
		std::shared_mutex	m_mutex;
		std::unordered_map<std::string, ResourceEntry> m_resources;
	};



	template <typename ResourceType>
	ResourceHandle ResourceManager::CreateAsync(std::string const& id, bool persistent)
	{
		std::unique_lock<std::shared_mutex> lock(m_mutex);

		auto [it, inserted] = m_resources.try_emplace(id, nullptr, persistent);

		ResourceEntry* entry = &it->second;

		m_threadPool.Enqueue(this, &ResourceManager::Create<ResourceType>, id, persistent);

		return ResourceHandle(entry);
	}

	template <typename ResourceType>
	ResourceHandle ResourceManager::Get(std::string id)
	{
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		auto it = m_resources.find(id);
		if (it != m_resources.end())
		{
			ResourceEntry* resource = &it->second;
			return ResourceHandle(resource);
		}
		else
		{
			return ResourceHandle(nullptr);
		}
	}

	template <typename ResourceType>
	void ResourceManager::Create(std::string const& id, bool persistent)
	{
		printf("creating %s\n", id.c_str());
		std::unique_ptr<ResourceType> ptr = std::make_unique<ResourceType>(id);
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		auto it = m_resources.find(id);
		if (it != m_resources.end())
		{
			it->second.m_resource = std::unique_ptr<IResource>(ptr.release());
		}
	}
}

#endif // !RESOURCE_MANAGER

