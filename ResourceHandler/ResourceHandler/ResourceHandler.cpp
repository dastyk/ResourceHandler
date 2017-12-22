#include "ResourceHandler.h"
#include <Profiler.h>
namespace ResourceHandler
{
	LoadJob Load(Utilz::GUID guid, Utilz::GUID type, Loader_Interface* loader, ResourcePassThrough* passThrough)
	{
		StartProfile;
		long exist = loader->Exist(guid, type);
		if (exist == 0)
			return { LoadStatus::NOT_FOUND | LoadStatus::FAILED };

		ResourceData data;
		auto result = loader->Read(guid, type, data);
		if (result < 0)
			return { LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED };

		if (passThrough)
		{
			//result = passThrough->passThrough();
			operator delete(data.data);
			if(result < 0)
				return { LoadStatus::PASS_THROUGH_FAILED | LoadStatus::FAILED };

		}

		return { LoadStatus::SUCCESS | LoadStatus::LOADED, data };
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
		StartProfile;
		if (auto find = passThroughs.find(type); find != passThroughs.end())
			return -1;

		passThroughs[type].memoryType = memoryType;
		passThroughs[type].passThrough = passThrough;
		return 0;
	}
	Resource ResourceHandler::LoadResource(Utilz::GUID guid, Utilz::GUID type)
	{
		StartProfile;
		size_t index;
		auto finalGUID = guid + type;
		if (auto resource = entries.find(finalGUID); resource.has_value())
		{
			index = resource->second;
		}
		else
		{
			index = entries.add(finalGUID);
			entries.get<GUID>(index) = guid;
			entries.get<Type>(index) = type;
			entries.get<Status>(index) = LoadStatus::NOT_LOADED;
			entries.get<RefCount>(index) = 0;
			entries.get<Data>(index) = ResourceData();
		}
		if (entries.get<Status>()[index] & LoadStatus::NOT_LOADED && !(entries.get<Status>()[index] & LoadStatus::LOADING))
		{
			entries.get<Status>()[index] |= LoadStatus::LOADING;
			entries.get<Future>()[index] = std::move(threadPool->Enqueue(Load, guid, type, loader, nullptr));
		}
		
		return Resource(finalGUID, this);
	}
	LoadStatus ResourceHandler::GetData(Utilz::GUID guid, ResourceData& data)
	{
		StartProfile;
		if (auto resource = entries.find(guid); resource.has_value())
		{
			auto& status = entries.get<Status>(resource->second);
			data = entries.get<Data>(resource->second);
			if (auto& f = entries.get<Future>(resource->second); f.valid())
			{
				auto result = f.get();
				result.status ^ LoadStatus::LOADING;
				if (result.status & LoadStatus::SUCCESS)
					status = status ^ LoadStatus::NOT_LOADED;
				status |= result.status;
				data = entries.get<Data>(resource->second) = result.data;
			}
			return status;
		}
		return LoadStatus::NOT_FOUND;
	}
	LoadStatus ResourceHandler::GetStatus(Utilz::GUID guid)
	{
		StartProfile;
		if (auto resource = entries.find(guid); resource.has_value())
		{
			auto& status = entries.get<Status>(resource->second);
			if (auto& f = entries.get<Future>(resource->second); f.valid())
			{
				auto result = f.get();
				result.status ^ LoadStatus::LOADING;
				if (result.status & LoadStatus::SUCCESS)
					status = status ^ LoadStatus::NOT_LOADED;
				status |= result.status;
				entries.get<Data>(resource->second) = result.data;
			}		
			return status;
		}
			
		return LoadStatus::NOT_FOUND;
	}
	void ResourceHandler::CheckIn(Utilz::GUID guid)
	{
		StartProfile;
		size_t index;
		if (auto resource = entries.find(guid); resource.has_value())
		{
			index = resource->second;
		}
		else
		{
			// TODO
			return;
		}
		entries.get<RefCount>(index)++;
		if (entries.get<Status>(index) & LoadStatus::NOT_LOADED && !(entries.get<Status>()[index] & LoadStatus::LOADING))
		{
			entries.get<Status>(index) |= LoadStatus::LOADING;
			entries.get<Future>(index) = std::move(threadPool->Enqueue(Load, entries.get<GUID>()[index], entries.get<Type>()[index], loader, nullptr));
		}
	}
	void ResourceHandler::CheckOut(Utilz::GUID guid)
	{
		StartProfile;
		if (auto resource = entries.find(guid); resource.has_value())
		{
			size_t index = resource->second;
			entries.get<RefCount>(index)--;
		}
	}
	size_t ResourceHandler::GetReferenceCount(Utilz::GUID guid)const
	{
		StartProfile;
		if (auto resource = entries.find(guid); resource.has_value())
		{
			size_t index = resource->second;
			return entries.getConst<RefCount>(index);
		}
		return  0;
	}
}