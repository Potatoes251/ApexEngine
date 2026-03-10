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

void ResourceManager::AddToGpuQueue(ResourceHandle<IResource> resource)
{
	std::lock_guard<std::mutex> lock(m_gpuQueueMutex);
	m_gpuUploadQueue.push(resource);
}