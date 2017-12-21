#include "ResourceHandler.h"

namespace ResourceHandler
{
	ResourceHandler::ResourceHandler()
	{
	}


	ResourceHandler::~ResourceHandler()
	{
	}
	Resource ResourceHandler::LoadResource(Utilz::GUID guid, Utilz::GUID type)
	{
		size_t index;
		if (auto resource = entries.find(guid); resource.has_value())
		{
			index = resource->second;
		}
		else
			index = entries.add(guid);

		entries.get<Promise>()[index] = Utilz::SuperPromise<LoadStatus>();
		entries.get<Promise>()[index].SetValue(LoadStatus::NOT_FOUND);
		return Resource(guid, this);
	}
	const ResourceData ResourceHandler::GetData(Utilz::GUID guid)const
	{
		return ResourceData();
	}
	LoadStatus ResourceHandler::GetStatus(Utilz::GUID guid)
	{
		if (auto resource = entries.find(guid); resource.has_value())
			return entries.get<Promise>(resource->second).GetFuture().Get();
		return LoadStatus::NOT_FOUND;
	}
}