#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_

#include <unordered_map>

#include <ResourceHandler\ResourceHandler_Interface.h> 
#include <Utilz\Sofa.h>
#include <Utilz\ThreadPool.h>
#include <windows.h> 
#include <map>
namespace ResourceHandler 
{
	struct Passthrough_Windows : public Passthrough_Info
	{
		Passthrough_Windows(const std::string& name) : name(name)
		{}
		std::string name;
		HINSTANCE lib;

	};
	struct LoadJob
	{
		LoadStatus status;
		ResourceDataVoid data;
	};

	class ResourceHandler_ : public ResourceHandler_Interface
	{
		friend class Resource;
	public:
		ResourceHandler_(FileSystem_Interface* loader, Utilz::ThreadPool* threadPool);
		~ResourceHandler_();
	//	long CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough) override;
	
		FILE_ERROR Initialize() override;
		void Shutdown() override;
	private:
		void LoadResource(const Resource& resource, bool invalid = false) override;
		LoadStatus GetData(const Resource& resource, ResourceDataVoid& data) override;
		LoadStatus PeekStatus(const Resource& resource)const override;
		void CheckIn(const Resource& resource) override;
		void CheckOut(const Resource& resource) override;
		uint32_t GetReferenceCount(const Resource& resource)const override;
		void Invalidate(const Resource& resource)override;

		FILE_ERROR  CreatePassthroughs();

		FileSystem_Interface * loader;
		Utilz::ThreadPool* threadPool;

		std::map<Utilz::GUID, Passthrough_Windows, Utilz::GUID::Compare> passthroughs;

		Utilz::Sofa<
				Utilz::GUID, Utilz::GUID::Hasher,
				ResourceDataVoid,
				LoadStatus,
				std::future<LoadJob>,
				uint32_t,
				Passthrough_Info*>
			entries;

		struct EntryNames
		{
			enum EntryNames_ : uint32_t
			{
				Key,
				Data,
				Status,
				Future,
				RefCount,
				Passthrough
			};
		};
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_