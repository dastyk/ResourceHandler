#include <Resource.h>
#include <ResourceHandler_Interface.h>

namespace ResourceHandler
{
	ResourceHandler::Resource::Resource()
	{
	}

	ResourceHandler::Resource::~Resource()
	{
	}

	LoadStatus ResourceHandler::Resource::Get()
	{
		return loadFuture.get();
	}

	void ResourceHandler::Resource::Unload()
	{
	}
}