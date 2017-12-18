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
	struct ResourceData
	{
		void* data;
		size_t size;
	};

	class ResourceHandler_Interface;
	class Resource
	{
	public:		
		Resource(Utilz::GUID guid, ResourceHandler_Interface* resourceHandler);
		DECLDIR ~Resource();

		DECLDIR LoadStatus GetStatus();
		DECLDIR const ResourceData GetData()const;
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