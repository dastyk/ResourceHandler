#include <Resource.h>
#include <ResourceHandler_Interface.h>

namespace ResourceHandler
{
	Resource::Resource(Utilz::GUID guid, ResourceHandler_Interface* resourceHandler)
		: resourceHandler(resourceHandler), myGUID(guid), checkInCount(0)
	{
	}

	Resource::~Resource()
	{
		if(checkInCount)
			resourceHandler->CheckOut(myGUID);
	}

	Resource::Resource(const Resource & other)
	{
		checkInCount = 0;
		resourceHandler = other.resourceHandler;
		myGUID = other.myGUID;
	}
	Resource::Resource(Resource && other) noexcept
	{
		checkInCount = other.checkInCount;
		resourceHandler = other.resourceHandler;
		myGUID = other.myGUID;
	}

	Resource & Resource::operator=(const Resource & other)
	{
		checkInCount = 0;
		resourceHandler = other.resourceHandler;
		myGUID = other.myGUID;
		return *this;
	}
	Resource& Resource::operator=(Resource && other) noexcept
	{
		checkInCount = other.checkInCount;
		resourceHandler = other.resourceHandler;
		myGUID = other.myGUID;
		return *this;
	}
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

	Utilz::GUID Resource::Type()const
	{
		return Utilz::GUID();
	}
}