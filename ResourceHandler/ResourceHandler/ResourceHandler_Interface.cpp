#include <ResourceHandler\ResourceHandler_Interface.h>
#include "ResourceHandler.h"

DECLDIR_RH_C ResourceHandler::ResourceHandler_Interface * ResourceHandler::CreateResourceHandler(Loader_Interface* loader, Utilz::ThreadPool* threadPool)
{
	return new ResourceHandler(loader, threadPool);
}
