#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_


#include <ResourceHandler_Interface.h>
#include "Sofa.h"
#include "SuperPromise.h"

namespace ResourceHandler 
{
	class ResourceHandler : public ResourceHandler_Interface
	{
		friend class Resource;
	public:
		ResourceHandler();
		~ResourceHandler();

		Resource LoadResource(Utilz::GUID guid, Utilz::GUID type) override;
	
	private:
		const ResourceData GetData(Utilz::GUID guid)const override;
		LoadStatus GetStatus(Utilz::GUID guid) override;


		Utilz::Sofa<
				Utilz::GUID, Utilz::GUID::Hasher,
				ResourceData,
				Utilz::SuperPromise<LoadStatus>>
			entries;

		enum EntryNames : uint32_t
		{
			GUID,
			Data,
			Promise
		};
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_