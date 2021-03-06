#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_

#include <unordered_map>

#include <ResourceHandler\ResourceHandler_Interface.h> 
#include <Sofa.h>
#include <ThreadPool.h>
#include <map>
namespace ResourceHandler 
{

	
	struct LoadJob
	{
		LoadStatus status;
		ResourceDataVoid data;
		ResourceDataVoid RAMData;
		uint64_t VRAMSize;
	};

	class ResourceHandler_ : public ResourceHandler_Interface
	{
		friend class Resource;
	public:
		ResourceHandler_(FileSystem_Interface* loader, Utilities::ThreadPool* threadPool);
		~ResourceHandler_();

		UERROR Initialize() override;
		void Shutdown() override;
		UERROR CreateType(const std::string& type, const Type_LoadInfo& info, bool force = false) override;
		UERROR AddType(Utilities::GUID type, const Type_Info& info) override;
	private:
		void LoadResource(const Resource& resource, bool invalid = false) override;
		LoadStatus GetData(const Resource& resource, ResourceDataVoid& data) override;
		LoadStatus PeekStatus(const Resource& resource)const override;
		void CheckIn(const Resource& resource) override;
		void CheckOut(const Resource& resource) override;
		uint32_t GetReferenceCount(const Resource& resource)const override;
		void Invalidate(const Resource& resource)override;

		UERROR  CreateTypes();

		FileSystem_Interface * loader;
		Utilities::ThreadPool* threadPool;

		std::map<Utilities::GUID, Type_Info, Utilities::GUID::Compare> types;
	
		Utilities::SofA::Vector::SofA<
				Utilities::GUID, Utilities::GUID::Hasher,
				ResourceDataVoid, // Parsed or not Parsed
				ResourceDataVoid, // RAM, data and size
				uint64_t,		 // VRAM, size
				LoadStatus,
				std::future<LoadJob>,
				uint32_t>
			entries;

		struct EntryNames
		{
			enum EntryNames_ : uint32_t
			{
				Key,
				Data,
				RAMData,
				VRAMSize,
				Status,
				Future,
				RefCount
			};
		};
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_