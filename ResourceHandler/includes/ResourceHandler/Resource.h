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

		DECLDIR Resource(const Resource& other);
		DECLDIR Resource(Resource&& other)noexcept;
		DECLDIR Resource& operator=(const Resource& other);
		DECLDIR Resource& operator=(Resource&& other)noexcept;

		DECLDIR LoadStatus PeekStatus();
		DECLDIR LoadStatus GetStatus();
		DECLDIR LoadStatus GetData(ResourceData& data);

		DECLDIR void CheckIn();
		DECLDIR void CheckOut();
		DECLDIR size_t GetReferenceCount()const;

		inline size_t GetCheckInCount()const
		{
			return checkInCount;
		}
		inline Utilz::GUID GUID()const
		{
			return myGUID;
		}
		DECLDIR Utilz::GUID Type()const;
	private:
		Utilz::GUID myGUID;
		size_t checkInCount;
		ResourceHandler_Interface* resourceHandler;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_