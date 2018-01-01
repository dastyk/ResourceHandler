#include <ResourceHandler\Resource.h>
#include <ResourceHandler\ResourceHandler_Interface.h>
#include "SecretPointer.h"

namespace ResourceHandler
{

	Resource::~Resource()
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		if (checkInCount)
		{
			resourceHandler->CheckOut(*this);
		}
			
	}

	DECLDIR_RH void Resource::Load()
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		resourceHandler->LoadResource(*this);
	}

	DECLDIR_RH void Resource::CheckIn()
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		if (++checkInCount == 1)
			resourceHandler->CheckIn(*this);
	}

	DECLDIR_RH void Resource::CheckOut()
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		if (--checkInCount == 0)
			resourceHandler->CheckOut(*this);
	}

	DECLDIR_RH uint32_t Resource::GetReferenceCount() const
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		return resourceHandler->GetReferenceCount(*this);
	}

	DECLDIR_RH LoadStatus Resource::GetData(ResourceDataVoid & data)
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		if (!checkInCount)
			CheckIn();
		return resourceHandler->GetData(*this, data);
	}

	DECLDIR_RH LoadStatus Resource::PeekStatus() const
	{
		_ASSERT_EXPR(resourceHandler, L"A Resource handler has not been created.");
		return resourceHandler->PeekStatus(*this);
	}

}