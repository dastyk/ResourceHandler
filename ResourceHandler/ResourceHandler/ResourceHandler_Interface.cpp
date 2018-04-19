#include <ResourceHandler\ResourceHandler_Interface.h>
#include "ResourceHandler.h"
#include <fstream>

DECLDIR_RH_C void DestroyThreadPool(Utilities::ThreadPool * tp)
{
	delete tp;
}

DECLDIR_RH_C void DestroyResourceHandler(ResourceHandler::ResourceHandler_Interface * rh)
{
	delete rh;
}

DECLDIR_RH_C Utilities::ThreadPool * CreateThreadPool(uint32_t numThreads)
{
	return new Utilities::ThreadPool(numThreads);
}

DECLDIR_RH_C ResourceHandler::ResourceHandler_Interface * CreateResourceHandler(ResourceHandler::FileSystem_Interface* loader, Utilities::ThreadPool* threadPool)
{
	return new ResourceHandler::ResourceHandler_(loader, threadPool);
}

DECLDIR_RH_C Utilities::Error ResourceHandler_CreateType(ResourceHandler::ResourceHandler_Interface * rh, const char * type, ResourceHandler::MemoryType memoryType, const char * passthrough)
{
	ResourceHandler::Type_LoadInfo info;
	info.memoryType = memoryType;

	if (passthrough && std::string(passthrough) != "")
	{
		std::ifstream pt(passthrough, std::ios::ate | std::ios::binary);

		info.passthrough.librarySize = pt.tellg();
		info.passthrough.library = new char[info.passthrough.librarySize];
		pt.seekg(0);
		pt.read(info.passthrough.library, info.passthrough.librarySize);
	}
	
	auto r = rh->CreateType(type, info, true);
	if (info.passthrough.library)
		delete[] info.passthrough.library;
	return r;
}
