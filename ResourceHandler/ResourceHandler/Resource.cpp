#include <ResourceHandler\Resource.h>
#include <ResourceHandler\ResourceHandler_Interface.h>
#include "SecretPointer.h"

namespace ResourceHandler
{
	Resource::Resource(Utilities::GUID guid, Utilities::GUID type) : myGUID(guid), myType(type), checkInCount(0) 
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		resourceHandler->LoadResource(*this);
	}
	Resource::Resource(Utilities::GUID guid, Utilities::GUID type, bool createAsInvalid) : myGUID(guid), myType(type), checkInCount(0)
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		resourceHandler->LoadResource(*this, createAsInvalid);

	}
	Resource::~Resource()
	{
	//	_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		if (resourceHandler && checkInCount)
		{
			resourceHandler->CheckOut(*this);
		}
			
	}


	DECLDIR_RH void Resource::CheckIn()
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		if (++checkInCount == 1)
			resourceHandler->CheckIn(*this);
	}

	DECLDIR_RH void Resource::CheckOut()
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		if (--checkInCount == 0)
			resourceHandler->CheckOut(*this);
	}

	DECLDIR_RH uint32_t Resource::GetReferenceCount() const
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		return resourceHandler->GetReferenceCount(*this);
	}

	DECLDIR_RH LoadStatus Resource::GetData(ResourceDataVoid & data)
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		if (!checkInCount)
			CheckIn();
		return resourceHandler->GetData(*this, data);
	}

	DECLDIR_RH LoadStatus Resource::PeekStatus() const
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		return resourceHandler->PeekStatus(*this);
	}

	DECLDIR_RH void Resource::Reset()
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		checkInCount = 0;
		resourceHandler->LoadResource(*this);
	}

	DECLDIR_RH void Resource::Reset( Resource resources[], uint32_t num)
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		for (size_t i = 0; i < num; i++)
		{
			resources[i].checkInCount = 0;
			resourceHandler->LoadResource(resources[i]);
		}
	}

	DECLDIR_RH void Resource::Invalidate() const
	{
		if (!resourceHandler)
			THROW_ERROR("A Resource handler has not been created.");
		resourceHandler->Invalidate(*this);
	}

}