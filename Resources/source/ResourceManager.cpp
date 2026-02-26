#include "ResourceManager.h"

using namespace Apex::Resources;

void ResourceManager::ProcessGpuUploads()
{
	while (true)
	{
		ResourceHandle resource;
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

ResourceHandle ResourceManager::Get(std::string const& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mapMutex);
	auto it = m_resources.find(id);
	if (it != m_resources.end() && it->second.m_resource)
	{
		ResourceEntry* resource = &it->second;
		return ResourceHandle(resource);
	}
	else
	{
		return ResourceHandle(nullptr);
	}
}

void ResourceManager::AddToGpuQueue(ResourceHandle resource)
{
	std::lock_guard<std::mutex> lock(m_gpuQueueMutex);
	m_gpuUploadQueue.push(resource);
}

void ResourceManager::Create(std::string const& id)
{
	ResourceHandle resource = Get(id);

	if (!resource.IsValid()) return;

	resource->LoadFromFile();

	// place the resource inside the already existing ResourceEntry
	resource->SetState(ResourceState::Loaded);
	AddToGpuQueue(resource);
}
