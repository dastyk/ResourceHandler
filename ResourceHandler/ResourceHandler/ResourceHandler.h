#ifndef _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define _RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#include <ResourceHandler_Interface.h>
namespace ResourceHandler 
{
	class ResourceHandler : public ResourceHandler_Interface
	{
	public:
		ResourceHandler();
		~ResourceHandler();

		Resource LoadResource(Utilz::GUID guid) override;
	};
}
#endif //_RESOURCE_HANDLER_RESOURCE_HANDLER_H_