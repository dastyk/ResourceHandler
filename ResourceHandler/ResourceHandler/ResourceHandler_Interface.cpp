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
	return new ResourceHandler::ResourceHandler(loader, threadPool);
}
