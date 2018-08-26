#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#include <GUID.h>

#include "LoadStatus.h"
#include "../DLLExport.h"
namespace ResourceHandler
{
	struct ResourceDataVoid
	{
		void* data = nullptr;
		uint64_t size = 0;
	};
	class Resource
	{
	public:		
		
		Resource(const Resource & other)noexcept
		{
			checkInCount = 0;
			myGUID = other.myGUID;
			myType = other.myType;
		}
		Resource(Resource && other) noexcept
		{
			checkInCount = other.checkInCount;
			myGUID = other.myGUID;
			myType = other.myType;
		}

		Resource & operator=(const Resource & other)noexcept
		{
			checkInCount = 0;
			myGUID = other.myGUID;
			myType = other.myType;
			return *this;
		}
		Resource& operator=(Resource && other) noexcept
		{
			checkInCount = other.checkInCount;
			myGUID = other.myGUID;
			myType = other.myType;
			return *this;
		}
	
		inline uint32_t GetCheckInCount()const
		{
			return checkInCount;
		}
		inline Utilities::GUID GUID()const
		{
			return myGUID;
		}
		inline Utilities::GUID Type()const
		{
			return myType;
		}
		
		DECLDIR_RH Resource(Utilities::GUID guid, Utilities::GUID type);
		DECLDIR_RH Resource(Utilities::GUID guid, Utilities::GUID type, bool createAsInvalid);
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
		Utilities::GUID myGUID;
		Utilities::GUID myType;
		uint32_t checkInCount;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_