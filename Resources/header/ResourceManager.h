#ifndef RESOURCE_MANAGER
#define RESOURCE_MANAGER

#include "ResourceHandle.h"
#include "ThreadPool.h"
 
#include <unordered_map>
#include <string>
#include <shared_mutex>


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
		ResourceHandle CreateAsync(std::string const& id, bool persistent = false);
		ResourceHandle Get(std::string id);

		void ProcessGpuUploads();
		void AddToGpuQueue(IResource*);

	private:
		void Create(std::string const& id);

		Core::ThreadPool&	m_threadPool;

		std::mutex				m_gpuQueueMutex;
		std::queue<IResource*>	m_gpuUploadQueue;

		std::shared_mutex		m_mapMutex;
		std::unordered_map<std::string, ResourceEntry> m_resources;
	};



	template <typename ResourceType>
	ResourceHandle ResourceManager::CreateAsync(std::string const& id, bool persistent)
	{
		std::unique_lock<std::shared_mutex> lock(m_mapMutex);

		auto [it, inserted] = m_resources.try_emplace(id, nullptr, persistent);
		ResourceEntry* entry = &it->second;

		// If already loading or loaded, do not enqueue again
		if (entry->m_resource && entry->m_resource->GetState() != ResourceState::Unloaded)
		{
			return ResourceHandle(entry);
		}

		entry->m_resource = std::make_unique<ResourceType>(id);
		entry->m_resource->SetState(ResourceState::Loading);
		//if (id == "Fragment.glsl|vertexShader.glsl")
		//	__debugbreak();
		m_threadPool.Enqueue(&ResourceManager::Create, this, id);

		return ResourceHandle(entry);
	}
}

#endif // !RESOURCE_MANAGER

