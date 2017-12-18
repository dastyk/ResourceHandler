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
		entires[guid];
		return Resource(guid, this);
	}
	const ResourceData ResourceHandler::GetData(Utilz::GUID guid)const
	{
		return ResourceData();
	}
	LoadStatus ResourceHandler::GetStatus(Utilz::GUID guid)
	{
		return LoadStatus();
	}
}