#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#include <GUID.h>

#include "LoadStatus.h"
#include "ResourceData.h"
#include "../DLLExport.h"
namespace ResourceHandler
{
	class Resource
	{
	public:		
		
		Resource(const Resource & other)noexcept
		{
			checkInCount = 0;
			myGUID = other.myGUID;
		}
		Resource(Resource && other) noexcept
		{
			checkInCount = other.checkInCount;
			myGUID = other.myGUID;
		}

		Resource & operator=(const Resource & other)noexcept
		{
			checkInCount = 0;
			myGUID = other.myGUID;
			return *this;
		}
		Resource& operator=(Resource && other) noexcept
		{
			checkInCount = other.checkInCount;
			myGUID = other.myGUID;
			return *this;
		}
	
		inline uint32_t GetCheckInCount()const
		{
			return checkInCount;
		}
		inline Utilz::GUID GUID()const
		{
			return myGUID;
		}
		inline Utilz::GUID Type()const
		{
			return myType;
		}
		
		DECLDIR_RH Resource(Utilz::GUID guid, Utilz::GUID type);
		DECLDIR_RH Resource(Utilz::GUID guid, Utilz::GUID type, bool createAsInvalid);
		DECLDIR_RH ~Resource();
		DECLDIR_RH void CheckIn();
		DECLDIR_RH void CheckOut();
		DECLDIR_RH uint32_t GetReferenceCount()const;
		DECLDIR_RH LoadStatus GetData(ResourceDataVoid& data);
		DECLDIR_RH LoadStatus PeekStatus()const;	
		DECLDIR_RH void Reset();
		DECLDIR_RH static void Reset( Resource resources[], uint32_t num);
		DECLDIR_RH void Invalidate()const;
	private:
		Utilz::GUID myGUID;
		Utilz::GUID myType;
		uint32_t checkInCount;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_