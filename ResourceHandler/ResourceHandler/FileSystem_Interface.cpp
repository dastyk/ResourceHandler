#include <ResourceHandler\FileSystem_Interface.h>
#include "BinaryFileSystem.h"
DECLDIR_RH_C ResourceHandler::FileSystem_Interface * CreateLoader(ResourceHandler::LoaderType t)
{
	switch (t)
	{
	case ResourceHandler::LoaderType::Binary:
		return new ResourceHandler::BinaryFileSystem();
		break;
	default:
		break;
	}
	return nullptr;
}

DECLDIR_RH_C long DestroyLoader(ResourceHandler::FileSystem_Interface * l)
{
	auto t = l->Shutdown();
	delete l;
	return t;
}

DECLDIR_RH_C long InitLoader_C(ResourceHandler::FileSystem_Interface* loader,const char * filePath, ResourceHandler::Mode mode)
{
	return loader->Init(filePath, mode);
}

DECLDIR_RH_C long Read_C(ResourceHandler::FileSystem_Interface * l, uint32_t guid, uint32_t type, void * data, uint64_t  size)
{
	return l->Read(guid, type, { data, size });
}

DECLDIR_RH_C long Destroy_C(ResourceHandler::FileSystem_Interface* l, uint32_t guid, uint32_t type)
{
	return l->Destroy(guid, type);
}

DECLDIR_RH_C long CreateS_C(ResourceHandler::FileSystem_Interface *l, const char * guid, const char * type, void * data, uint64_t size)
{
	return l->Create(std::string(guid), std::string(type), { data, size });
}

DECLDIR_RH_C long CreateFromFile_C(ResourceHandler::FileSystem_Interface * l, const char * path, const char * guid, const char * type)
{
	return l->CreateFromFile(path, guid, type);
}

DECLDIR_RH_C long Exist_C(ResourceHandler::FileSystem_Interface * l, uint32_t guid, uint32_t type)
{
	return l->Exist(guid, type);
}

DECLDIR_RH_C long ExistS_C(ResourceHandler::FileSystem_Interface * l, const char * guid, const char * type)
{
	return l->Exist(std::string(guid), std::string(type));
}

DECLDIR_RH_C long ReadS_C(ResourceHandler::FileSystem_Interface * l, const char * guid, const char * type, void * data, uint64_t  size)
{
	return l->Read(std::string(guid), std::string(type), { data, size });
}

DECLDIR_RH_C long WriteS_C(ResourceHandler::FileSystem_Interface *l, const char * guid, const char * type, void * data, uint64_t size)
{
	return l->Write(std::string(guid), std::string(type), { data, size });
}

DECLDIR_RH_C long DestroyS_C(ResourceHandler::FileSystem_Interface *l, const char * guid, const char * type)
{
	return l->Destroy(std::string(guid), std::string(type));
}

DECLDIR_RH_C long Defrag_C(ResourceHandler::FileSystem_Interface * l)
{
	return l->Defrag();
}

DECLDIR_RH_C long GetSizeOfFile_C(ResourceHandler::FileSystem_Interface *l, uint32_t guid, uint32_t type, uint64_t * size)
{
	return l->GetSizeOfFile(guid, type, *size);
}

DECLDIR_RH_C long GetSizeOfFileS_C(ResourceHandler::FileSystem_Interface *l, const char * guid, const char * type, uint64_t * size)
{
	return  l->GetSizeOfFile(std::string(guid), std::string(type), *size);
}

DECLDIR_RH_C uint32_t GetNumberOfFiles_C(ResourceHandler::FileSystem_Interface * l)
{
	return l->GetNumberOfFiles();
}

DECLDIR_RH_C uint32_t GetNumberOfTypes_C(ResourceHandler::FileSystem_Interface * l)
{
	return l->GetNumberOfTypes();
}

DECLDIR_RH_C uint64_t GetTotalSizeOfAllFiles_C(ResourceHandler::FileSystem_Interface * l)
{
	return l->GetTotalSizeOfAllFiles();
}
DECLDIR_RH_C float GetFragmentationRatio_C(ResourceHandler::FileSystem_Interface * l)
{
	return l->GetFragmentationRatio();
}
DECLDIR_RH_C long GetFiles_C(ResourceHandler::FileSystem_Interface * l, ResourceHandler::FILE_C* files, uint32_t numFiles)
{
	return l->GetFiles(files, numFiles);
}

DECLDIR_RH_C long GetFile_C(ResourceHandler::FileSystem_Interface * l, ResourceHandler::FILE_C * file, const char* guid, const char* type)
{
	return l->GetFile(*file,std::string( guid), std::string(type));
}

DECLDIR_RH_C uint32_t GetNumberOfFilesOfType_C(ResourceHandler::FileSystem_Interface * l, const char * type)
{
	return l->GetNumberOfFilesOfType(std::string(type));
}

DECLDIR_RH_C long GetFilesOfType_C(ResourceHandler::FileSystem_Interface * l, const char * type, ResourceHandler::FILE_C * files, uint32_t numFiles)
{
	return l->GetFilesOfType(std::string(type), files, numFiles);
}
