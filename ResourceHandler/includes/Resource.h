#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif


#include <GUID.h>

#include "LoadStatus.h"
#include "ResourceData.h"
namespace ResourceHandler
{
	class ResourceHandler_Interface;
	class Resource
	{
	public:		
		Resource(Utilz::GUID guid, ResourceHandler_Interface* resourceHandler);
		DECLDIR ~Resource();

		DECLDIR LoadStatus Status();
		DECLDIR const ResourceData GetData()const;
		DECLDIR void Unload();
		inline Utilz::GUID GUID()const
		{
			return myGUID;
		}
		inline Utilz::GUID Type()const
		{
			return myType;
		}
	private:
		Utilz::GUID myGUID;
		Utilz::GUID myType;
		ResourceHandler_Interface* resourceHandler;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_