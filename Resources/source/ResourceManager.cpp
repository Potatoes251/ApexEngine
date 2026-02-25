#include "ResourceManager.h"

using namespace Apex::Resources;

void ResourceManager::ProcessGpuUploads()
{
	std::lock_guard<std::mutex> lock(m_gpuQueueMutex);
	while (!m_gpuUploadQueue.empty())
	{
		IResource* resource = m_gpuUploadQueue.front();
		m_gpuUploadQueue.pop();
		if (resource->GetState() == ResourceState::Failed) continue;
		resource->UploadToGpu();
		if (resource->GetState() == ResourceState::Failed) continue;
		resource->SetState(ResourceState::Uploaded);
	}
}

void ResourceManager::AddToGpuQueue(IResource* resource)
{
	std::lock_guard<std::mutex> lock(m_gpuQueueMutex);
	m_gpuUploadQueue.push(resource);
}

ResourceHandle ResourceManager::Get(std::string id)
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

void ResourceManager::Create(std::string const& id)
{
	printf("creating %s\n", id.c_str());

	IResource* resource = nullptr;

	{
		std::lock_guard<std::shared_mutex> lock(m_mapMutex);

		auto it = m_resources.find(id);

		if (it == m_resources.end()) return;

		resource = it->second.m_resource.get();
	}

	resource->LoadFromFile();

	// place the resource inside the already existing ResourceEntry
	resource->SetState(ResourceState::Loaded);
	AddToGpuQueue(resource);
}