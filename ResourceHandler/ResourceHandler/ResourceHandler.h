#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#include <ResourceHandler_Interface.h>
#include "Sofa.h"
#include <future>

namespace ResourceHandler 
{
	class ResourceHandler : public ResourceHandler_Interface
	{
	public:
		ResourceHandler();
		~ResourceHandler();

		Resource LoadResource(Utilz::GUID guid) override;
		const ResourceData GetData(Utilz::GUID guid)const override;
		LoadStatus GetStatus(Utilz::GUID guid) override;
	private:
		Utilz::Sofa<Utilz::GUID, Utilz::GUID::Hasher,
			ResourceData,
		std::promise<LoadStatus>,
		std::future<LoadStatus>> entires;
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_