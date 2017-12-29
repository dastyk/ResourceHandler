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
		Resource(Utilz::GUID guid, Utilz::GUID type) : myGUID(guid), myType(type), checkInCount(0) {}
		void Reset()
		{
			checkInCount = 0;
		}
		Resource::Resource(const Resource & other)noexcept
		{
			checkInCount = 0;
			myGUID = other.myGUID;
		}
		Resource::Resource(Resource && other) noexcept
		{
			checkInCount = other.checkInCount;
			myGUID = other.myGUID;
		}

		Resource & Resource::operator=(const Resource & other)noexcept
		{
			checkInCount = 0;
			myGUID = other.myGUID;
			return *this;
		}
		Resource& Resource::operator=(Resource && other) noexcept
		{
			checkInCount = other.checkInCount;
			myGUID = other.myGUID;
			return *this;
		}
		DECLDIR_RH ~Resource();
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
		void operator=(ResourceHandler_Interface* rh);
		Resource& operator++();
		Resource& operator--();
	private:
		Utilz::GUID myGUID;
		Utilz::GUID myType;
		uint32_t checkInCount;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_