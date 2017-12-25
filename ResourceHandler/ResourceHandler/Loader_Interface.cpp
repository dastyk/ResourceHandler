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

DECLDIR long DestroyLoader(ResourceHandler::Loader_Interface * l)
{
	auto t = l->Shutdown();
	delete l;
	return t;
}

DECLDIR long InitLoader_C(ResourceHandler::Loader_Interface* loader,const char * filePath, ResourceHandler::Mode mode)
{
	return loader->Init(filePath, mode);
}

DECLDIR long Read_C(ResourceHandler::Loader_Interface * l, uint32_t guid, uint32_t type, void * data, uint64_t  size)
{
	return l->Read(guid, type, { data, size });
}

DECLDIR long Destroy_C(ResourceHandler::Loader_Interface* l, uint32_t guid, uint32_t type)
{
	return l->Destroy(guid, type);
}

DECLDIR long CreateS_C(ResourceHandler::Loader_Interface *l, const char * guid, const char * type, void * data, uint64_t size)
{
	return l->Create(std::string(guid), std::string(type), { data, size });
}

DECLDIR long Exist_C(ResourceHandler::Loader_Interface * l, uint32_t guid, uint32_t type)
{
	return l->Exist(guid, type);
}

DECLDIR long ExistS_C(ResourceHandler::Loader_Interface * l, const char * guid, const char * type)
{
	return l->Exist(std::string(guid), std::string(type));
}

DECLDIR long ReadS_C(ResourceHandler::Loader_Interface * l, const char * guid, const char * type, void * data, uint64_t  size)
{
	return l->Read(std::string(guid), std::string(type), { data, size });
}

DECLDIR long DestroyS_C(ResourceHandler::Loader_Interface *l, const char * guid, const char * type)
{
	return l->Destroy(std::string(guid), std::string(type));
}

DECLDIR long Defrag_C(ResourceHandler::Loader_Interface * l)
{
	return l->Defrag();
}

DECLDIR long GetSizeOfFile_C(ResourceHandler::Loader_Interface *l, uint32_t guid, uint32_t type, uint64_t * size)
{
	return l->GetSizeOfFile(guid, type, *size);
}

DECLDIR long GetSizeOfFileS_C(ResourceHandler::Loader_Interface *l, const char * guid, const char * type, uint64_t * size)
{
	return  l->GetSizeOfFile(std::string(guid), std::string(type), *size);
}

DECLDIR uint32_t GetNumberOfFiles_C(ResourceHandler::Loader_Interface * l)
{
	return l->GetNumberOfFiles();
}

DECLDIR uint32_t GetNumberOfTypes_C(ResourceHandler::Loader_Interface * l)
{
	return l->GetNumberOfTypes();
}

DECLDIR uint64_t GetTotalSizeOfAllFiles_C(ResourceHandler::Loader_Interface * l)
{
	return l->GetTotalSizeOfAllFiles();
}
#include <objbase.h>
DECLDIR long GetFiles(ResourceHandler::Loader_Interface * l, FILE_C ** files)
{
	std::vector<ResourceHandler::File> dfiles;
	long r = l->GetFiles(dfiles);
	if (dfiles.size())
	{

		*files = (FILE_C*)::CoTaskMemAlloc(dfiles.size());
		for (size_t i = 0; i < dfiles.size(); i++)
		{
			(*files)[i].guid = dfiles[i].guid;
			(*files)[i].type = dfiles[i].type;
			(*files)[i].guid_str = (char*)::CoTaskMemAlloc(dfiles[i].guid_str.size() + 1);
			strcpy((*files)[i].guid_str, dfiles[i].guid_str.c_str());
			(*files)[i].type_str = (char*)::CoTaskMemAlloc(dfiles[i].type_str.size() + 1);
			strcpy((*files)[i].type_str, dfiles[i].type_str.c_str());
		}
	}
	return r;
}
