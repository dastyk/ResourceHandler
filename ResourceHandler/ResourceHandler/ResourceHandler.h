#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_

#include <unordered_map>

#include <ResourceHandler_Interface.h>
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
		ResourceData data;
	};

	class ResourceHandler : public ResourceHandler_Interface
	{
		friend class Resource;
	public:
		ResourceHandler(Loader_Interface* loader, Utilz::ThreadPool* threadPool);
		~ResourceHandler();
		long CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough) override;
		Resource LoadResource(Utilz::GUID guid, Utilz::GUID type) override;
	
	private:
		LoadStatus GetData(Utilz::GUID guid, ResourceData& data) override;
		LoadStatus GetStatus(Utilz::GUID guid) override;
		void CheckIn(Utilz::GUID guid) override;
		void CheckOut(Utilz::GUID guid) override;
		size_t GetReferenceCount(Utilz::GUID guid)const override;

		Loader_Interface * loader;
		Utilz::ThreadPool* threadPool;


		Utilz::Sofa<
				Utilz::GUID, Utilz::GUID::Hasher,
				Utilz::GUID,
				Utilz::GUID,
				ResourceData,
				LoadStatus,
				std::future<LoadJob>,
				size_t>
			entries;

		enum EntryNames : uint32_t
		{
			Key,
			GUID,
			Type,
			Data,
			Status,
			Future,
			RefCount
		};

	
		std::unordered_map<Utilz::GUID, ResourcePassThrough, Utilz::GUID::Hasher> passThroughs;
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_