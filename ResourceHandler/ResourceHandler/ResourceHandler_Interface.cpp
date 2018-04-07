#include <ResourceHandler\ResourceHandler_Interface.h>
#include "ResourceHandler.h"

DECLDIR_RH_C void DestroyThreadPool(Utilz::ThreadPool * tp)
{
	delete tp;
}

DECLDIR_RH_C void DestroyResourceHandler(ResourceHandler::ResourceHandler_Interface * rh)
{
	delete rh;
}

DECLDIR_RH_C Utilz::ThreadPool * CreateThreadPool(uint32_t numThreads)
{
	return new Utilz::ThreadPool(numThreads);
}

DECLDIR_RH_C ResourceHandler::ResourceHandler_Interface * CreateResourceHandler(ResourceHandler::FileSystem_Interface* loader, Utilz::ThreadPool* threadPool)
{
	return new ResourceHandler::ResourceHandler_(loader, threadPool);
}

DECLDIR_RH_C ResourceHandler::File_Error ResourceHandler_CreateType(ResourceHandler::ResourceHandler_Interface * rh, const char * type, ResourceHandler::MemoryType memoryType, const char * passthrough)
{
	ResourceHandler::Type_LoadInfo info;
	info.memoryType = memoryType;

	if (passthrough && std::string(passthrough) != "")
	{
		std::ifstream pt(passthrough, std::ios::ate | std::ios::binary);
		ResourceHandler::Type_LoadInfo pti;

		pti.passthrough.librarySize = pt.tellg();
		pti.passthrough.library = new char[pti.passthrough.librarySize];
		pt.seekg(0);
		pt.read(pti.passthrough.library, pti.passthrough.librarySize);
	}
	
	auto r = rh->CreateType(type, info, true);
	if (info.passthrough.library)
		delete[] info.passthrough.library;
	return r;
}
