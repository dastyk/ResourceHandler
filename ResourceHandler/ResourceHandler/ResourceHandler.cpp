#include "ResourceHandler.h"

namespace ResourceHandler
{
	ResourceHandler::ResourceHandler(Loader_Interface* loader, Utilz::ThreadPool* threadPool)
		: loader(loader) , threadPool(threadPool)
	{
		_ASSERT(loader); 
		_ASSERT(threadPool);
	}


	ResourceHandler::~ResourceHandler()
	{
		for (size_t i = 0; i < entries.size(); i++)
			operator delete(entries.get<Data>()[i].data);
	}
	long ResourceHandler::CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough)
	{
		if (auto find = passThroughs.find(type); find != passThroughs.end())
			return -1;

		passThroughs[type].memoryType = memoryType;
		passThroughs[type].passThrough = passThrough;
		return 0;
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

		long exist = loader->Exist(guid, type);
		entries.get<Promise>()[index] = Utilz::SuperPromise<LoadStatus>();
		if (exist == 0)
			entries.get<Promise>()[index].SetValue(LoadStatus::NOT_FOUND);
		else
		{
			auto result = loader->Read(guid, type, entries.get<Data>()[index]);
			if (result == -1)
				entries.get<Promise>()[index].SetValue(LoadStatus::COULD_NOT_LOAD);
			else
				entries.get<Promise>()[index].SetValue(LoadStatus::SUCCESS);
		}
		
		return Resource(guid, type, this);
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