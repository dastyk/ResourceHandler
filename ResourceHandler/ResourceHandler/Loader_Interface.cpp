#include <Loader_Interface.h>
#include "DirectoryLoader.h"
#include "BinaryLoader.h"
DECLDIR ResourceHandler::Loader_Interface * ResourceHandler::CreateLoader(LoaderType t)
{
	switch (t)
	{
	case ResourceHandler::LoaderType::Directory:
		break;
	case ResourceHandler::LoaderType::Binary:
		return new BinaryLoader();
		break;
	default:
		break;
	}
	return nullptr;
}
