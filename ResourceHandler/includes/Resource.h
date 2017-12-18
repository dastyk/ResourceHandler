#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif


#include "LoadStatus.h"
#include <GUID.h>

namespace ResourceHandler
{
	class ResourceHandler_Interface;
	class Resource
	{
	public:		
		Resource(ResourceHandler_Interface* resourceHandler);
		DECLDIR ~Resource();

		DECLDIR LoadStatus GetStatus();
		DECLDIR void Unload();
		inline Utilz::GUID GetGUID()const
		{
			return myGUID;
		}

	private:
		Utilz::GUID myGUID;
		ResourceHandler_Interface* resourceHandler;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_