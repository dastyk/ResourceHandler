#include "ResourceHandler.h"

namespace ResourceHandler
{
	ResourceHandler::ResourceHandler()
	{
	}


	ResourceHandler::~ResourceHandler()
	{
	}
	Resource ResourceHandler::LoadResource(Utilz::GUID guid)
	{
		if (auto resource = entires.find(guid); resource.has_value())
		{
			return Resource(this);
		}
		return Resource(this);
	}
}