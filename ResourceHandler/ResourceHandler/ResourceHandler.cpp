#include "ResourceHandler.h"

namespace ResourceHandler
{
	LoadStatus Load(Utilz::GUID guid, Utilz::GUID type, Loader_Interface* loader, ResourcePassThrough* passThrough)
	{
		long exist = loader->Exist(guid, type);
		if (exist == 0)
			return LoadStatus::NOT_FOUND | LoadStatus::FAILED;

		ResourceData data;
		auto result = loader->Read(guid, type, data);
		if (result < 0)
			return LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED;

		if (passThrough)
		{
			//result = passThrough->passThrough();
			operator delete(data.data);
			if(result < 0)
				return LoadStatus::PASS_THROUGH_FAILED | LoadStatus::FAILED;

		}

		return LoadStatus::SUCCESS | LoadStatus::LOADED;
	}

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