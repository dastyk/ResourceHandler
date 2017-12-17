#ifndef _RESOURCE_HANDLER_INTERFACE_H_
#define _RESOURCE_HANDLER_INTERFACE_H_
#include "Resource.h"
#include <GUID.h>

namespace ResourceHandler
{
	class ResourceHandler_Interface
	{
	public:
		virtual	Resource LoadResource(Utilz::GUID guid) = 0;
	protected:
		ResourceHandler_Interface() {};
		virtual ~ResourceHandler_Interface() {};
	};


#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
	DECLDIR ResourceHandler_Interface* CreateResourceHandler();
}

#endif // _RESOURCE_HANDLER_INTERFACE_H_