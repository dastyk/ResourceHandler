#include <ResourceHandler\Resource.h>
#include <ResourceHandler\ResourceHandler_Interface.h>
#include "SecretPointer.h"

namespace ResourceHandler
{
	//Resource::Resource(Utilz::GUID guid, ResourceHandler_Interface* resourceHandler)
	//	: resourceHandler(resourceHandler), myGUID(guid), checkInCount(0)
	//{
	//}

	Resource::~Resource()
	{
		_ASSERT_EXPR(resourceHandler, "A resource never got coupled with a resource handler");
		if (checkInCount)
		{
			checkInCount = 1;
			resourceHandler->CheckOut(*this);
		}
			
	}

	DECLDIR_RH void Resource::Load()
	{
		resourceHandler->LoadResource(*this);
	}

	DECLDIR_RH void Resource::CheckIn()
	{
		if (++checkInCount == 1)
			resourceHandler->CheckIn(*this);
	}

	DECLDIR_RH void Resource::CheckOut()
	{
		if (--checkInCount == 0)
			resourceHandler->CheckOut(*this);
	}

	DECLDIR_RH uint32_t Resource::GetReferenceCount() const
	{
		return resourceHandler->GetReferenceCount(*this);
	}

	DECLDIR_RH LoadStatus Resource::GetData(ResourceDataVoid & data)
	{
		return resourceHandler->GetData(*this, data);
	}

	DECLDIR_RH LoadStatus Resource::PeekStatus() const
	{
		return resourceHandler->PeekStatus(*this);
	}



/*
	LoadStatus Resource::PeekStatus()
	{
		return LoadStatus();
	}

	LoadStatus Resource::GetStatus()
	{
		if(checkInCount)
			return resourceHandler->GetStatus(myGUID);
		return LoadStatus::NOT_CHECKED_IN;
	}

	LoadStatus Resource::GetData(ResourceData& data)
	{
		if (checkInCount)
			return resourceHandler->GetData(myGUID,data);
		return LoadStatus::NOT_CHECKED_IN;
	}

	void Resource::CheckIn()
	{
		if(!checkInCount)
			resourceHandler->CheckIn(myGUID);
		checkInCount++;
		return void();
	}

	void Resource::CheckOut()
	{
		if (!checkInCount)
			resourceHandler->CheckOut(myGUID);
		checkInCount--;
		return void();
	}

	size_t Resource::GetReferenceCount() const
	{
		return resourceHandler->GetReferenceCount(myGUID);
	}

	Resource::Resource(Utilz::GUID guid, Utilz::GUID type)
	{
	}

	Utilz::GUID Resource::Type()const
	{
		return Utilz::GUID();
	}*/
}