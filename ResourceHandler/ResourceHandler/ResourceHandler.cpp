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


		ResourceDataVoid data;
		auto result = loader->GetSizeOfFile(guid, type, data.size);
		if(result < 0)
			return { LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED };
		data.data = operator new((size_t(data.size)));
		result = loader->Read(guid, type, data);
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
	void ResourceHandler::LoadResource(Resource& resource)
	{
		StartProfile;
		size_t index;
		auto finalGUID = resource.GUID() + resource.Type();
		if (auto findR = entries.find(finalGUID); findR.has_value())
		{
			index = findR->second;
		}
		else
		{
			index = entries.add(finalGUID);
			entries.get<Status>(index) = LoadStatus::NOT_LOADED;
			entries.get<RefCount>(index) = 0;
			entries.get<Data>(index) = ResourceDataVoid();
		}
		if (entries.get<Status>()[index] & LoadStatus::NOT_LOADED && !(entries.get<Status>()[index] & LoadStatus::LOADING))
		{
			entries.get<Status>()[index] |= LoadStatus::LOADING;
			entries.get<Future>()[index] = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr));
		}

		resource = this;
	}
	LoadStatus ResourceHandler::GetData(const Resource& resource, ResourceDataVoid& data)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			auto& status = entries.get<Status>(findRe->second);
			data = entries.get<Data>(findRe->second);
			if (auto& f = entries.get<Future>(findRe->second); f.valid())
			{
				auto result = f.get();
				result.status = result.status ^ LoadStatus::LOADING;
				if (result.status & LoadStatus::SUCCESS)
					status = status ^ LoadStatus::NOT_LOADED;
				status |= result.status;
				data = entries.get<Data>(findRe->second) = result.data;
			}
			return status;
		}
		return LoadStatus::NOT_FOUND;
	}
	LoadStatus ResourceHandler::GetStatus(const Resource& resource)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			auto& status = entries.get<Status>(findRe->second);
			if (auto& f = entries.get<Future>(findRe->second); f.valid())
			{
				auto result = f.get();
				result.status = result.status ^ LoadStatus::LOADING;
				if (result.status & LoadStatus::SUCCESS)
					status = status ^ LoadStatus::NOT_LOADED;
				status |= result.status;
				entries.get<Data>(findRe->second) = result.data;
			}		
			return status;
		}
			
		return LoadStatus::NOT_FOUND;
	}
	void ResourceHandler::CheckIn(Resource& resource)
	{
		StartProfile;
		size_t index;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			index = findRe->second;
		}
		else
		{
			index = entries.add(resource.GUID() + resource.Type());
			entries.get<Status>(index) = LoadStatus::NOT_LOADED;
			entries.get<RefCount>(index) = 0;
			entries.get<Data>(index) = ResourceDataVoid();
		}
		if (resource.GetCheckInCount() == 0)
		{
			resource = this;
			entries.get<RefCount>(index)++;
		}
			
		++resource;
		if (entries.get<Status>(index) & LoadStatus::NOT_LOADED && !(entries.get<Status>()[index] & LoadStatus::LOADING))
		{
			entries.get<Status>(index) |= LoadStatus::LOADING;
			entries.get<Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr));
		}
	}
	void ResourceHandler::CheckOut(Resource& resource)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			size_t index = findRe->second;
			if(resource.GetCheckInCount() == 1)
				entries.get<RefCount>(index)--;
			--resource;
		}
	}
	uint32_t ResourceHandler::GetReferenceCount(const Resource& resource)const
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			size_t index = findRe->second;
			return entries.getConst<RefCount>(index);
		}
		return  0;
	}
}