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
		return Resource(this);
	}
}