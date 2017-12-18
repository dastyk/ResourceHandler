#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#include <ResourceHandler_Interface.h>
#include "Sofa.h"
namespace ResourceHandler 
{
	class ResourceHandler : public ResourceHandler_Interface
	{
	public:
		ResourceHandler();
		~ResourceHandler();

		Resource LoadResource(Utilz::GUID guid) override;

	private:
		struct ResourceData
		{
			void* data;
			size_t size;
		};
		Utilz::Sofa<Utilz::GUID, Utilz::GUID::Hasher,
			ResourceData> entires;
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_