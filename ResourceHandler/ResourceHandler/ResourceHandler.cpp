#include "ResourceHandler.h"
#include <Profiler.h>
#include "SecretPointer.h"
#include <Error.h>
#include <Windows.h>
#include <filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem;

ResourceHandler::ResourceHandler_Interface* resourceHandler = nullptr;




namespace ResourceHandler
{

	struct Passthrough_Windows
	{
		Passthrough_Windows(const std::string& name) : name(name)
		{}
		std::string name;
		HINSTANCE lib;

	};

	std::vector<Passthrough_Windows> ptws;

	LoadJob Load(Utilities::GUID guid, Utilities::GUID type, FileSystem_Interface* loader, const Type_Info* typeInfo, LoadStatus extraFlag)
	{

		StartProfile;
		if (!loader->Exist(guid, type))
			return { LoadStatus::NOT_FOUND | LoadStatus::FAILED | LoadStatus::NOT_LOADED };


		ResourceDataVoid data;
		auto result = loader->GetSizeOfFile(guid, type, data.size);
		if(result.hash != "Success"_hash)
			return { LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED | LoadStatus::NOT_LOADED };
		data.data = operator new((size_t(data.size)));
		result = loader->Read(guid, type, data);
		if (result.hash != "Success"_hash)
			return { LoadStatus::COULD_NOT_LOAD | LoadStatus::FAILED | LoadStatus::NOT_LOADED };

		if (typeInfo && typeInfo->passthrough.Parse)
		{
			ResourceDataVoid parsedData;
			auto parseResult = typeInfo->passthrough.Parse(guid, data.data, data.size, &parsedData.data, &parsedData.size);
			if (parseResult < 0)
				return { LoadStatus::PASS_THROUGH_FAILED | LoadStatus::FAILED | LoadStatus::NOT_LOADED };

			operator delete(data.data);
			data = parsedData;
		}

		return { LoadStatus::SUCCESS | LoadStatus::LOADED | extraFlag, data };
	}

	ResourceHandler_::ResourceHandler_(FileSystem_Interface* loader, Utilities::ThreadPool* threadPool)
		: loader(loader) , threadPool(threadPool)
	{
		_ASSERT(loader); 
		_ASSERT(threadPool);
		resourceHandler = this;
	}


	ResourceHandler_::~ResourceHandler_()
	{
		for (size_t i = 0; i < entries.size(); i++)
			if (auto findType = types.find(entries.get<EntryNames::Key>(i)); findType != types.end())
				if(findType->second.passthrough.Destroy)
					findType->second.passthrough.Destroy(entries.get<EntryNames::Key>()[i], entries.get<EntryNames::Data>()[i].data, entries.get<EntryNames::Data>()[i].size);
			else
				operator delete(entries.get<EntryNames::Data>()[i].data);
		for (auto& ptw : ptws)
			FreeLibrary(ptw.lib);
		ptws.clear();
		resourceHandler = nullptr;
		
	}
	UERROR ResourceHandler_::Initialize()
	{
		return CreateTypes();
	}
	void ResourceHandler_::Shutdown()
	{
	}

	UERROR ResourceHandler_::CreateType(const std::string& type, const Type_LoadInfo& info, bool force)
	{

		if (auto findType = types.find(type); findType != types.end())
			if (!force)
				RETURN_ERROR("Type already exists");

		Type_Info typeInfo;
		if (info.passthrough.library != nullptr)
		{
			ptws.push_back(Passthrough_Windows(type + ".pat"));
			auto& ptw = ptws.back();
			typeInfo.passthrough = Passthrough_Info();

			std::ofstream file(ptw.name, std::ios::trunc | std::ios::binary);
			if (!file.is_open())
				RETURN_ERROR("Could not open passthrough file");

			file.write(info.passthrough.library, info.passthrough.librarySize);
			file.close();

			ptw.lib = LoadLibrary(ptw.name.c_str());
			if (ptw.lib == NULL)
				RETURN_ERROR("Could not load passthrough library");

			typeInfo.passthrough.Parse = (Passthrough_Parse_PROC)GetProcAddress(ptw.lib, "Parse");
			if (typeInfo.passthrough.Parse == NULL)
				RETURN_ERROR("Could not load 'Parse' function from passthrough library");

			typeInfo.passthrough.Destroy = (Passthrough_Destroy_PROC)GetProcAddress(ptw.lib, "Destroy");
			if (typeInfo.passthrough.Destroy == NULL)
				RETURN_ERROR("Could not load 'Destroy' function from passthrough library");
		}

		PASS_IF_ERROR(loader->CreateFromCallback(type, "Type", [&](std::ostream* file) {
			file->write((char*)&info, sizeof(info));
			file->write(info.passthrough.library, info.passthrough.librarySize);
			return true;
		}));


		types.emplace(type, typeInfo);

		RETURN_SUCCESS;
	}

	UERROR ResourceHandler_::AddType(Utilities::GUID type, const Type_Info & info)
	{
		types.emplace(type, info);
	}
	
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
			entries.get<EntryNames::Status>(index) = LoadStatus::NOT_LOADED;
			entries.get<EntryNames::RefCount>(index) = 0;
			entries.get<EntryNames::Data>(index) = ResourceDataVoid();
		}
		if (entries.get<EntryNames::Status>(index) & LoadStatus::INVALIDATED || invalid)
		{
			entries.get<EntryNames::Status>(index) |= LoadStatus::LOADING;
			entries.get<EntryNames::Status>(index) = (entries.get<EntryNames::Status>(index) ^ LoadStatus::INVALIDATED) | LoadStatus::VALIDATING;
			entries.get<EntryNames::Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::INVALIDATED));
		}
		else if ((entries.get<EntryNames::Status>(index) & LoadStatus::NOT_LOADED && !(entries.get<EntryNames::Status>()[index] & LoadStatus::LOADING)))
		{
			entries.get<EntryNames::Status>(index) |= LoadStatus::LOADING;
			if (auto findType = types.find(resource.Type()); findType != types.end())
				entries.get<EntryNames::Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, &findType->second, LoadStatus::NONE));
			else
				entries.get<EntryNames::Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::NONE));
		}
	}
	LoadStatus ResourceHandler_::GetData(const Resource& resource, ResourceDataVoid& data)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			auto& status = entries.get<EntryNames::Status>(findRe->second);
			data = entries.get<EntryNames::Data>(findRe->second);

			redo:
			if (auto& f = entries.get<EntryNames::Future>(findRe->second); f.valid())
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
				
				
				auto& ddata = entries.get<EntryNames::Data>(findRe->second);
				if (ddata.data != nullptr)
					operator delete(ddata.data);
				data = ddata = result.data;
			}

			if (status & LoadStatus::INVALIDATED)
			{
				entries.get<EntryNames::Status>(findRe->second) |= LoadStatus::LOADING;
				entries.get<EntryNames::Status>(findRe->second) = (entries.get<EntryNames::Status>(findRe->second) ^ LoadStatus::INVALIDATED) | LoadStatus::VALIDATING;
				entries.get<EntryNames::Future>(findRe->second) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::INVALIDATED));
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
			return entries.getConst<EntryNames::Status>(findRe->second);
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
			entries.get<EntryNames::Status>(index) = LoadStatus::NOT_LOADED;
			entries.get<EntryNames::RefCount>(index) = 0;
			entries.get<EntryNames::Data>(index) = ResourceDataVoid();
		}

		entries.get<EntryNames::RefCount>(index)++;


		if (entries.get<EntryNames:: Status>(index) & LoadStatus::INVALIDATED)
		{
			entries.get<EntryNames::Status>(index) |= LoadStatus::LOADING;
			entries.get<EntryNames::Status>(index) = (entries.get<EntryNames::Status>(index) ^ LoadStatus::INVALIDATED) | LoadStatus::VALIDATING;
			entries.get<EntryNames::Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::INVALIDATED));
		}
		else if ((entries.get<EntryNames::Status>(index) & LoadStatus::NOT_LOADED && !(entries.get<EntryNames::Status>()[index] & LoadStatus::LOADING)))
		{
			entries.get<EntryNames::Status>(index) |= LoadStatus::LOADING;
			if (auto findType = types.find(resource.GUID()); findType != types.end())
				entries.get<EntryNames::Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, &findType->second, LoadStatus::NONE));
			else
				entries.get<EntryNames::Future>(index) = std::move(threadPool->Enqueue(Load, resource.GUID(), resource.Type(), loader, nullptr, LoadStatus::NONE));
		}
		
	}
	void ResourceHandler_::CheckOut(const Resource& resource)
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			size_t index = findRe->second;
			entries.get<EntryNames::RefCount>(index)--;
		}
	}
	uint32_t ResourceHandler_::GetReferenceCount(const Resource& resource)const
	{
		StartProfile;
		if (auto findRe = entries.find(resource.GUID() + resource.Type()); findRe.has_value())
		{
			size_t index = findRe->second;
			return entries.getConst<EntryNames::RefCount>(index);
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
	

	UERROR  ResourceHandler_::CreateTypes()
	{
		std::vector<File> loaded_types;
	
		loader->GetFilesOfType("Type", loaded_types);
		for (auto& type : loaded_types)
		{
			ResourceData<Type_LoadInfo> data;
			PASS_IF_ERROR(loader->GetSizeOfFile(type.guid, type.type, data.GetVoid().size));
			data.GetVoid().data = operator new((size_t(data.GetVoid().size)));
			PASS_IF_ERROR(loader->Read(type.guid, type.type, data));
			Type_Info typeInfo;
			typeInfo.memoryType = data->memoryType;
		
			if(data->passthrough.library != nullptr)
			{
				ptws.push_back(Passthrough_Windows(type.guid_str + ".pat"));
				auto& ptw = ptws.back();
				typeInfo.passthrough = Passthrough_Info();
				if (!fs::exists(type.guid_str + ".pat"))
				{
					std::ofstream file(ptw.name, std::ios::trunc | std::ios::binary);
					if (!file.is_open())
						RETURN_ERROR("Could not open passthrough file");

					auto pp = data.GetExtra();

					file.write(pp, data->passthrough.librarySize);
					file.close();
					
					operator delete(data.GetVoid().data);
				}

				ptw.lib = LoadLibrary(ptw.name.c_str());
				if (ptw.lib == NULL)
					RETURN_ERROR("Could not load passthrough library");

				typeInfo.passthrough.Parse = (Passthrough_Parse_PROC)GetProcAddress(ptw.lib, "Parse");
				if (typeInfo.passthrough.Parse == NULL)
					RETURN_ERROR("Could not load 'Parse' function from passthrough library");

				typeInfo.passthrough.Destroy = (Passthrough_Destroy_PROC)GetProcAddress(ptw.lib, "Destroy");
				if (typeInfo.passthrough.Destroy == NULL)
					RETURN_ERROR("Could not load 'Destroy' function from passthrough library");
			}

		
		
			types.emplace(type.guid, typeInfo);

		}

		RETURN_SUCCESS;
	}
}