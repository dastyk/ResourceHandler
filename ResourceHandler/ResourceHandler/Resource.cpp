#include <Resource.h>
#include <ResourceHandler_Interface.h>

namespace ResourceHandler
{
	ResourceHandler::Resource::Resource(ResourceHandler_Interface* resourceHandler) 
		: resourceHandler(resourceHandler)
	{
	}

	ResourceHandler::Resource::~Resource()
	{
	}

	LoadStatus ResourceHandler::Resource::GetStatus()
	{
		return LoadStatus::FAILED;
	}

	void ResourceHandler::Resource::Unload()
	{
	}
}