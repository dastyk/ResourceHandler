#include "ResourceHandler.h"
#include <Profiler.h>
#include "SecretPointer.h"
#include <File_Error.h>
#include <Windows.h>
#include <File_Error.h>
#include <filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem;

ResourceHandler::ResourceHandler_Interface* resourceHandler = nullptr;
namespace ResourceHandler
{
	LoadJob Load(Utilz::GUID guid, Utilz::GUID type, FileSystem_Interface* loader, ResourcePassThrough* passThrough, LoadStatus extraFlag)
	{
		StartProfile;
		if (!loader->Exist(guid, type))
			return { LoadStatus::NOT_FOUND | LoadStatus::FAILED | LoadStatus::NOT_LOADED };


		ResourceDataVoid data;
		auto result = loader->GetSizeOfFile(guid, type, data.size);
		if(result.errornr < 0)
			return { LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED | LoadStatus::NOT_LOADED };
		data.data = operator new((size_t(data.size)));
		result = loader->Read(guid, type, data);
		if (result.errornr < 0)
			return { LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED | LoadStatus::NOT_LOADED };

		if (passThrough)
		{
			//result = passThrough->passThrough();
			operator delete(data.data);
			if(result.errornr < 0)
				return { LoadStatus::PASS_THROUGH_FAILED | LoadStatus::FAILED | LoadStatus::NOT_LOADED };

		}

		return { LoadStatus::SUCCESS | LoadStatus::LOADED | extraFlag, data };
	}

	ResourceHandler_::ResourceHandler_(FileSystem_Interface* loader, Utilz::ThreadPool* threadPool)
		: loader(loader) , threadPool(threadPool)
	{
		_ASSERT(loader); 
		_ASSERT(threadPool);
		resourceHandler = this;
	}


	ResourceHandler_::~ResourceHandler_()
	{
		for (size_t i = 0; i < entries.size(); i++)
			operator delete(entries.get<Data>()[i].data);
	}
	FILE_ERROR ResourceHandler_::Initialize()
	{
		return CreatePassthroughs();
	}
	void ResourceHandler_::Shutdown()
	{
	}
	//long ResourceHandler_::CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough)
	//{
	//	StartProfile;
	//	if (auto find = passThroughs.find(type); find != passThroughs.end())
	//		return -1;

	//	passThroughs[type].memoryType = memoryType;
	//	passThroughs[type].passThrough = passThrough;
	//	return 0;
	//}
	void ResourceHandler_::LoadResource(const Resource& resource, bool invalid)
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
		if (entries.get<Status>(index) & LoadStatus::INVALIDATED || invalid)
		{
			entries.get<Status>(index) |= LoadStatus::LOADING;
			entries.get<Status>(index) = (entries.get<Status>(index) ^ LoadStatus::INVALIDATED) | LoadStatus::VALIDATING;
			entries.get<Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::INVALIDATED));
		}
		else if ((entries.get<Status>(index) & LoadStatus::NOT_LOADED && !(entries.get<Status>()[index] & LoadStatus::LOADING)))
		{
			entries.get<Status>(index) |= LoadStatus::LOADING;
			entries.get<Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::NONE));
		}
	}
	LoadStatus ResourceHandler_::GetData(const Resource& resource, ResourceDataVoid& data)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			auto& status = entries.get<Status>(findRe->second);
			data = entries.get<Data>(findRe->second);

			redo:
			if (auto& f = entries.get<Future>(findRe->second); f.valid())
			{
				auto result = f.get();
				if (result.status & LoadStatus::INVALIDATED)
					status = result.status ^ LoadStatus::INVALIDATED;
				else if (status & LoadStatus::INVALIDATED)
					status = result.status | LoadStatus::INVALIDATED;
				else if (status & LoadStatus::VALIDATING)
					status = result.status | LoadStatus::VALIDATING;
				else
					status = result.status;
				
				
				auto& ddata = entries.get<Data>(findRe->second);
				if (ddata.data != nullptr)
					operator delete(ddata.data);
				data = ddata = result.data;
			}

			if (status & LoadStatus::INVALIDATED)
			{
				entries.get<Status>(findRe->second) |= LoadStatus::LOADING;
				entries.get<Status>(findRe->second) = (entries.get<Status>(findRe->second) ^ LoadStatus::INVALIDATED) | LoadStatus::VALIDATING;
				entries.get<Future>(findRe->second) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::INVALIDATED));
				goto redo;
			}

			return status;
		}
		return LoadStatus::NOT_FOUND | LoadStatus::NOT_LOADED | LoadStatus::FAILED;
	}
	LoadStatus ResourceHandler_::PeekStatus(const Resource& resource)const
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			return entries.getConst<Status>(findRe->second);
		}
			
		return LoadStatus::NOT_FOUND |LoadStatus::NOT_LOADED | LoadStatus::FAILED;
	}
	void ResourceHandler_::CheckIn(const Resource& resource)
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

		entries.get<RefCount>(index)++;


		if (entries.get<Status>(index) & LoadStatus::INVALIDATED)
		{
			entries.get<Status>(index) |= LoadStatus::LOADING;
			entries.get<Status>(index) = (entries.get<Status>(index) ^ LoadStatus::INVALIDATED) | LoadStatus::VALIDATING;
			entries.get<Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::INVALIDATED));
		}
		else if ((entries.get<Status>(index) & LoadStatus::NOT_LOADED && !(entries.get<Status>()[index] & LoadStatus::LOADING)))
		{
			entries.get<Status>(index) |= LoadStatus::LOADING;
			entries.get<Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::NONE));
		}
		
	}
	void ResourceHandler_::CheckOut(const Resource& resource)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			size_t index = findRe->second;
			entries.get<RefCount>(index)--;
		}
	}
	uint32_t ResourceHandler_::GetReferenceCount(const Resource& resource)const
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			size_t index = findRe->second;
			return entries.getConst<RefCount>(index);
		}
		return  0;
	}
	void ResourceHandler_::Invalidate(const Resource & resource)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			entries.get<EntryNames::Status>(findRe->second) |= LoadStatus::INVALIDATED;
		}
	}
	

	const File_Error&  ResourceHandler_::CreatePassthroughs()
	{
		std::vector<File> pt;
	
		loader->GetFilesOfType("Passthrough", pt);
		for (auto& passT : pt)
		{
			Passthrough_Info pti(passT.guid_str + ".pat");
			if (!fs::exists(pti.name))
			{
				ResourceDataVoid data;
				PASS_IF_FILE_ERROR(loader->Read(passT.guid, passT.type, data));
				
				
				std::ofstream file(pti.name, std::ios::trunc);
				if (!file.is_open())
					RETURN_FILE_ERROR_C("Could not open passthrough file");
				file.write((char*)data.data, data.size);
				file.close();

				
			}

			pti.lib = LoadLibrary(pti.name.c_str());
			if (pti.lib == NULL)
				RETURN_FILE_ERROR_C("Could not load passthrough library");
			
			pti.Parse = (Passthrough_Info::Parse_PROC)GetProcAddress(pti.lib, "Parse");
			if(pti.Parse == NULL)
				RETURN_FILE_ERROR_C("Could not load Parse function from passthrough library");
			
			pti.Destroy = (Passthrough_Info::Destroy_PROC)GetProcAddress(pti.lib, "Destroy");
			if (pti.Destroy == NULL)
				RETURN_FILE_ERROR_C("Could not load Destroy function from passthrough library");
		
			passThroughs.emplace(passT.guid, pti);

		}

		RETURN_FILE_SUCCESS;
	}
}