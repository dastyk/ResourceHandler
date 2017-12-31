#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_

#include <unordered_map>

#include <ResourceHandler\ResourceHandler_Interface.h> 
#include <Utilz\Sofa.h>

namespace ResourceHandler 
{
	struct ResourcePassThrough
	{
		ResourceHandler_Interface::PassThroughCallback passThrough;
		MemoryType memoryType;
	};
	struct LoadJob
	{
		LoadStatus status;
		ResourceDataVoid data;
	};

	class ResourceHandler : public ResourceHandler_Interface
	{
		friend class Resource;
	public:
		ResourceHandler(Loader_Interface* loader, Utilz::ThreadPool* threadPool);
		~ResourceHandler();
		long CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough) override;
	
	private:
		void LoadResource(const Resource& resource) override;
		LoadStatus GetData(const Resource& resource, ResourceDataVoid& data) override;
		LoadStatus PeekStatus(const Resource& resource)const override;
		void CheckIn(const Resource& resource) override;
		void CheckOut(const Resource& resource) override;
		uint32_t GetReferenceCount(const Resource& resource)const override;



		Loader_Interface * loader;
		Utilz::ThreadPool* threadPool;


		Utilz::Sofa<
				Utilz::GUID, Utilz::GUID::Hasher,
				ResourceDataVoid,
				LoadStatus,
				std::future<LoadJob>,
				uint32_t>
			entries;

		enum EntryNames : uint32_t
		{
			Key,
			Data,
			Status,
			Future,
			RefCount
		};

	
		std::unordered_map<Utilz::GUID, ResourcePassThrough, Utilz::GUID::Hasher> passThroughs;
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_