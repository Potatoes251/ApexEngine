#include "ResourceManager.h"

using namespace Apex::Resources;

void ResourceManager::ProcessGpuUploads()
{
	while (true)
	{
		ResourceHandle<IResource> resource;
		{
			std::lock_guard<std::mutex> lock(m_gpuQueueMutex);
			if (m_gpuUploadQueue.empty()) break;

			resource = m_gpuUploadQueue.front();
			m_gpuUploadQueue.pop();
		}

		if (resource.IsFailed()) continue;
		resource->UploadToGpu();
		if (resource.IsFailed()) continue;
		resource->SetState(ResourceState::Uploaded);
	}
}

void Apex::Resources::ResourceManager::CollectGarbage(float deltaTime)
{
	std::unique_lock lock(m_mapMutex);

	for (auto it = m_resources.begin(); it != m_resources.end(); )
	{
		auto& [id, entry] = *it;

		if (entry->m_refCount.load() == 0 && !entry->m_persistent)
		{
			entry->m_timeSinceUnused += deltaTime;

			if (entry->m_timeSinceUnused > 5.0f)
			{
				printf("Releasing resource %s\n", id.c_str());
				entry->m_resource.reset();
				it = m_resources.erase(it);
				continue;
			}
		}
		else
		{
			entry->m_timeSinceUnused = 0.f;
		}

		++it;
	}
}

void ResourceManager::AddToGpuQueue(ResourceHandle<IResource> resource)
{
	std::lock_guard<std::mutex> lock(m_gpuQueueMutex);
	m_gpuUploadQueue.push(resource);
}