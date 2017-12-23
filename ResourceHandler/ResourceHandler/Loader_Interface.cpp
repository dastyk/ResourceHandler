#include <Loader_Interface.h>
#include "BinaryLoader.h"
DECLDIR ResourceHandler::Loader_Interface * CreateLoader(ResourceHandler::LoaderType t)
{
	switch (t)
	{
	case ResourceHandler::LoaderType::Binary:
		return new ResourceHandler::BinaryLoader();
		break;
	default:
		break;
	}
	return nullptr;
}

DECLDIR long InitLoader(ResourceHandler::Loader_Interface* loader,const char * filePath, ResourceHandler::Mode mode)
{
	return loader->Init(filePath, mode);
}
