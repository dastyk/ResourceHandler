#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif


#include "LoadStatus.h"

namespace ResourceHandler
{
	class ResourceHandler_Interface;
	class DECLDIR Resource
	{
	public:		
		Resource(ResourceHandler_Interface* resourceHandler);
		~Resource();

		LoadStatus GetStatus();
		void Unload();
	private:
		ResourceHandler_Interface* resourceHandler;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_