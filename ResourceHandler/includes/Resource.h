#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
#include <future>

#include <GUID.h>

#include "LoadStatus.h"

namespace ResourceHandler
{
	class ResourceHandler_Interface;
	DECLDIR class Resource
	{
	public:
		Resource();
		~Resource();

		LoadStatus Get();

		void Unload();
	private:
		Utilz::GUID myGUID;
		ResourceHandler_Interface* resourceHandler;
		std::future<LoadStatus> loadFuture;
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_