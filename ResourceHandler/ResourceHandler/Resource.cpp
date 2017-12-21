#include <Resource.h>
#include <ResourceHandler_Interface.h>

namespace ResourceHandler
{
	ResourceHandler::Resource::Resource(Utilz::GUID guid, ResourceHandler_Interface* resourceHandler)
		: resourceHandler(resourceHandler), myGUID(guid)
	{
	}

	ResourceHandler::Resource::~Resource()
	{
	}

	LoadStatus ResourceHandler::Resource::Status()
	{
		return resourceHandler->GetStatus(myGUID);
	}

	const ResourceData Resource::GetData() const
	{
		return resourceHandler->GetData(myGUID);
	}

	void ResourceHandler::Resource::Unload()
	{
	}
}