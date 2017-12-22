#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_

#include <unordered_map>

#include <ResourceHandler_Interface.h>
#include <Utilz\Sofa.h>
#include <Utilz\SuperPromise.h>

namespace ResourceHandler 
{
	class ResourceHandler : public ResourceHandler_Interface
	{
		friend class Resource;
	public:
		ResourceHandler(Loader_Interface* loader, Utilz::ThreadPool* threadPool);
		~ResourceHandler();
		long CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough) override;
		Resource LoadResource(Utilz::GUID guid, Utilz::GUID type) override;
	
	private:
		const ResourceData GetData(Utilz::GUID guid)const override;
		LoadStatus GetStatus(Utilz::GUID guid) override;


		Loader_Interface * loader;
		Utilz::ThreadPool* threadPool;

		Utilz::Sofa<
				Utilz::GUID, Utilz::GUID::Hasher,
				ResourceData,
				Utilz::SuperPromise<LoadStatus>,
				size_t>
			entries;

		enum EntryNames : uint32_t
		{
			GUID,
			Data,
			Promise,
			RefCount
		};

		struct ResourcePassThrough
		{
			PassThroughCallback passThrough;
			MemoryType memoryType;
		};
		std::unordered_map<Utilz::GUID, ResourcePassThrough, Utilz::GUID::Hasher> passThroughs;
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_