#ifndef _RESOURCE_HANDLER_LOADER_INTERFACE_H_
#define _RESOURCE_HANDLER_LOADER_INTERFACE_H_
#include <string>
#include <vector>

#include <GUID.h>

#include "ResourceData.h"

namespace ResourceHandler
{
	enum class Mode
	{
		EDIT,
		READ
	};

	struct File
	{
		Utilz::GUID guid;
		Utilz::GUID type;
		std::string guid_str;
		std::string type_str;
	};

	class Loader_Interface
	{
	public:
		virtual ~Loader_Interface() {};
		virtual long Init(const char* filePath, Mode mode) noexcept = 0;
		virtual long Shutdown() noexcept = 0; 

		virtual long FindType(Utilz::GUID guid, Utilz::GUID& type)const noexcept = 0;
		virtual long FindNameAndType(Utilz::GUID guid, Utilz::GUID& name, Utilz::GUID& type)const noexcept = 0;
		virtual long Exist(Utilz::GUID guid, Utilz::GUID type)const noexcept = 0;
		virtual long Read(Utilz::GUID guid, Utilz::GUID type,const ResourceData& data) noexcept = 0;

		virtual long Create(const std::string& guid,const std::string& type, const ResourceData& data) noexcept = 0;
		virtual long Destroy(Utilz::GUID, Utilz::GUID type) noexcept = 0;

		virtual long Defrag()noexcept = 0;

		virtual long GetFilesOfType(Utilz::GUID type, std::vector<File>& files) const noexcept = 0;
		virtual long GetFiles(std::vector<File>& files)const noexcept = 0;
		virtual long GetSizeOfFile(Utilz::GUID guid, Utilz::GUID type, uint64_t& size)const noexcept = 0;
		virtual uint32_t GetNumberOfFiles()const noexcept = 0;
		virtual uint32_t GetNumberOfTypes()const noexcept = 0;
		virtual uint64_t GetTotalSizeOfAllFiles()const noexcept = 0;
	protected:
		Loader_Interface() {};
	};

	enum class LoaderType : uint32_t
	{
		Binary
	};

}
#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR  __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
extern "C" DECLDIR ResourceHandler::Loader_Interface* CreateLoader(ResourceHandler::LoaderType);
extern "C" DECLDIR long DestroyLoader(ResourceHandler::Loader_Interface*);
extern "C" DECLDIR long InitLoader_C(ResourceHandler::Loader_Interface*,const char* filePath, ResourceHandler::Mode);
extern "C" DECLDIR long Read_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type, void* data, uint64_t size);
extern "C" DECLDIR long Destroy_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type);
extern "C" DECLDIR long CreateS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, void* data, uint64_t size);
extern "C" DECLDIR long Exist_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type);
extern "C" DECLDIR long ExistS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type);
extern "C" DECLDIR long ReadS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, void* data, uint64_t size);
extern "C" DECLDIR long DestroyS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type);
extern "C" DECLDIR long Defrag_C(ResourceHandler::Loader_Interface*);
extern "C" DECLDIR long GetSizeOfFile_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type, uint64_t* size);
extern "C" DECLDIR long GetSizeOfFileS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, uint64_t* size);
extern "C" DECLDIR uint32_t GetNumberOfFiles_C(ResourceHandler::Loader_Interface*);
extern "C" DECLDIR uint32_t GetNumberOfTypes_C(ResourceHandler::Loader_Interface*);
extern "C" DECLDIR uint64_t GetTotalSizeOfAllFiles_C(ResourceHandler::Loader_Interface*);


struct FILE_C
{
	uint32_t guid;
	uint32_t type;
	char* guid_str;
	char* type_str;
};
extern "C" DECLDIR long GetFiles_C(ResourceHandler::Loader_Interface* l,
	FILE_C** files,
	uint32_t * numFiles);
#endif
