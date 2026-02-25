#include "ResourceHandle.h"

using namespace Apex::Resources;

ResourceHandle::ResourceHandle(ResourceEntry* resourceEntry)
	: m_resourceEntry(resourceEntry)
{
	if (m_resourceEntry)
	{
		m_resourceEntry->m_refCount.fetch_add(1);
		printf("%d\n", m_resourceEntry->m_refCount.load());
	}
}

ResourceHandle::ResourceHandle(ResourceHandle const& other)
	: m_resourceEntry(other.m_resourceEntry)
{
	m_resourceEntry->m_refCount.fetch_add(1);
	printf("%d\n", m_resourceEntry->m_refCount.load());
}

ResourceHandle& ResourceHandle::operator=(ResourceHandle const& other)
{
	if (this != &other)
	{
		if (m_resourceEntry) m_resourceEntry->m_refCount.fetch_sub(1);

		m_resourceEntry = other.m_resourceEntry;
		m_resourceEntry->m_refCount.fetch_add(1);
		printf("%d\n", m_resourceEntry->m_refCount.load());
	}
	return *this;
}

ResourceHandle::~ResourceHandle()
{
	if (!m_resourceEntry)
		return;

	m_resourceEntry->m_refCount.fetch_sub(1);

	if (m_resourceEntry->m_refCount.load() == 0 && !m_resourceEntry->m_persistent)
	{
		m_resourceEntry->m_resource.reset();
		printf("resource destroyed\n");
	}
}

bool ResourceHandle::IsValid() const
{
	return m_resourceEntry;
}

bool ResourceHandle::IsReady() const
{
	return	m_resourceEntry && 
			m_resourceEntry->m_resource &&
			m_resourceEntry->m_resource->GetState() == ResourceState::Uploaded;
}

bool ResourceHandle::IsFailed() const
{
	return	m_resourceEntry && 
			m_resourceEntry->m_resource &&
			m_resourceEntry->m_resource->GetState() == ResourceState::Failed;
}

IResource* ResourceHandle::operator->() const
{
	return m_resourceEntry ? m_resourceEntry->m_resource.get() : nullptr;
}