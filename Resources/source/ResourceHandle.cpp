#include "ResourceHandle.h"

using namespace Apex::Resources;

ResourceHandle::ResourceHandle(ResourceEntry* resourceEntry)
	: m_resourceEntry(resourceEntry)
{
	if (m_resourceEntry)
	{
		(m_resourceEntry->m_refCount)++;
		printf("%d\n", m_resourceEntry->m_refCount.load());
	}
}

ResourceHandle::ResourceHandle(ResourceHandle const& other)
	: m_resourceEntry(other.m_resourceEntry)
{
	(m_resourceEntry->m_refCount)++;
	printf("%d\n", m_resourceEntry->m_refCount.load());
}

ResourceHandle& ResourceHandle::operator=(ResourceHandle const& other)
{
	if (this != &other)
	{
		if (m_resourceEntry) (m_resourceEntry->m_refCount)--;

		m_resourceEntry = other.m_resourceEntry;
		(m_resourceEntry->m_refCount)++;
		printf("%d\n", m_resourceEntry->m_refCount.load());
	}
	return *this;
}

ResourceHandle::~ResourceHandle()
{
	if (!m_resourceEntry)
		return;

	--(m_resourceEntry->m_refCount);

	if (m_resourceEntry->m_refCount == 0 && !m_resourceEntry->m_persistent)
	{
		m_resourceEntry->m_resource.reset();
		m_resourceEntry = nullptr;
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
			m_resourceEntry->m_resource->getState() == ResourceState::Uploaded;
}

IResource* ResourceHandle::operator->() const
{
	return m_resourceEntry ? m_resourceEntry->m_resource.get() : nullptr;
}