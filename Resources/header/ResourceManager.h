#ifndef RESOURCE_MANAGER
#define RESOURCE_MANAGER

#include "ResourceHandle.h"
#include "ThreadPool.h"
 
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <cassert>

namespace Apex::Resources
{
	// restrict template type for CreateAsync function
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
		ResourceHandle<ResourceType> CreateAsync(std::string const& id, bool persistent = false);
		template <typename ResourceType>
		ResourceHandle<ResourceType> Get(std::string const& id);
		template<typename T>
		std::vector<ResourceHandle<T>> GetAll();

		void ProcessGpuUploads();

		void SetRHI(Rendering::IRHI* rhi) { m_rhi = rhi; }

	private:
		void AddToGpuQueue(ResourceHandle<IResource>);
		template<typename ResourceType>
		void Create(std::string const& id);

		Rendering::IRHI* m_rhi = nullptr;

		Core::ThreadPool&	m_threadPool;

		std::mutex					m_gpuQueueMutex;
		std::queue<ResourceHandle<IResource>>	m_gpuUploadQueue;

		std::shared_mutex		m_mapMutex;
		std::unordered_map<std::string, std::unique_ptr<ResourceEntry>> m_resources;
	};



	template <typename ResourceType>
	ResourceHandle<ResourceType> ResourceManager::CreateAsync(std::string const& id, bool persistent)
	{
		assert(m_rhi != nullptr && "rhi must be set in the ResourceManager before starting to load resources");
		std::unique_lock<std::shared_mutex> lock(m_mapMutex);
		std::cout << "Loading: " << id << std::endl;
		auto [it, inserted] = m_resources.try_emplace(id, std::make_unique<ResourceEntry>(nullptr, persistent));
		ResourceEntry* entry = it->second.get();

		// If already loading or loaded, do not enqueue again
		if (entry->m_resource && entry->m_resource->GetState() != ResourceState::Unloaded)
		{
			return ResourceHandle<ResourceType>(entry);
		}

		entry->m_resource = std::make_unique<ResourceType>(id);
		entry->m_resource->SetState(ResourceState::Loading);
		entry->m_resource->SetRhi(m_rhi);

		m_threadPool.Enqueue(&ResourceManager::Create<ResourceType>, this, id);

		return ResourceHandle<ResourceType>(entry);
	}

	template <typename ResourceType>
	ResourceHandle<ResourceType> ResourceManager::Get(std::string const& id)
	{
		std::shared_lock<std::shared_mutex> lock(m_mapMutex);
		auto it = m_resources.find(id);
		if (it != m_resources.end() && it->second.get()->m_resource)
		{
			ResourceEntry* resource = it->second.get();
			return ResourceHandle<ResourceType>(resource);
		}
		else
		{
			return ResourceHandle<ResourceType>(nullptr);
		}
	}

	template <typename ResourceType>
	void ResourceManager::Create(std::string const& id)
	{
		ResourceHandle resource = Get<IResource>(id);

		if (!resource.IsValid()) return;

		resource->LoadFromFile();

		resource->SetState(ResourceState::Loaded);
		AddToGpuQueue(resource);
	}

	template<typename T>
	std::vector<ResourceHandle<T>> ResourceManager::GetAll()
	{
		std::shared_lock lock(m_mapMutex);

		std::vector<ResourceHandle<T>> result;

		for (auto& [id, entry] : m_resources)
		{
			if (!entry->m_resource)
				continue;

			if (auto* casted = dynamic_cast<T*>(entry->m_resource.get()))
			{
				result.emplace_back(entry.get());
			}
		}

		return result;
	}
}

#endif // !RESOURCE_MANAGER

