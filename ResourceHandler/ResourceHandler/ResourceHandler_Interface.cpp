#include <ResourceHandler_Interface.h>
#include "ResourceHandler.h"

DECLDIR ResourceHandler::ResourceHandler_Interface * ResourceHandler::CreateResourceHandler()
{
	return new ResourceHandler();
}
