#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#include <GUID.h>

#include "LoadStatus.h"
#include "ResourceData.h"
#include "../DLLExport.h"
namespace ResourceHandler
{
	class ResourceHandler_Interface;
	class Resource
	{
	public:		
		Resource(Utilz::GUID guid, ResourceHandler_Interface* resourceHandler);
		DECLDIR_RH ~Resource();

		DECLDIR_RH Resource(const Resource& other);
		DECLDIR_RH Resource(Resource&& other)noexcept;
		DECLDIR_RH Resource& operator=(const Resource& other);
		DECLDIR_RH Resource& operator=(Resource&& other)noexcept;

		DECLDIR_RH LoadStatus PeekStatus();
		DECLDIR_RH LoadStatus GetStatus();
		DECLDIR_RH LoadStatus GetData(ResourceData& data);

		DECLDIR_RH void CheckIn();
		DECLDIR_RH void CheckOut();
		DECLDIR_RH size_t GetReferenceCount()const;

		inline size_t GetCheckInCount()const
		{
			return checkInCount;
		}
		inline Utilz::GUID GUID()const
		{
			return myGUID;
		}
		DECLDIR_RH Utilz::GUID Type()const;
	private:
		Utilz::GUID myGUID;
		size_t checkInCount;
		ResourceHandler_Interface* resourceHandler;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_