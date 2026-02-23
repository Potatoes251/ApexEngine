#ifndef RESOURCE_MANAGER
#define RESOURCE_MANAGER

#include "IResource.h"
 
#include <unordered_map>
#include <string>

namespace Apex::Resources
{
	class ResourceManager
	{
	public:
		ResourceManager();

	private:

		std::unordered_map<std::string, IResource> m_resources;
	};
}

#endif // !RESOURCE_MANAGER

