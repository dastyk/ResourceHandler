#ifndef _RESOURCE_HANDLER_LOADER_INTERFACE_H_
#define _RESOURCE_HANDLER_LOADER_INTERFACE_H_
#include <string>
#include <vector>
#include <functional>

#include <GUID.h>

#include "ResourceData.h"
#include "../DLLExport.h"

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
	struct FILE_C
	{
		uint32_t guid;
		uint32_t type;
		const char* guid_str;
		const char* type_str;
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
		virtual long Read(Utilz::GUID guid, Utilz::GUID type, const ResourceDataVoid& data) noexcept = 0;

		virtual long Create(const std::string& guid,const std::string& type, const ResourceDataVoid& data) noexcept = 0;
		virtual long CreateFromFile(const char* filePath, const std::string& guid, const std::string& type) noexcept = 0;
		virtual long CreateFromCallback(const std::string& guid, const std::string& type, const std::function<bool(std::ostream* file)>& function)noexcept = 0;
		
		virtual long Write(Utilz::GUID guid, Utilz::GUID type, const ResourceDataVoid& data)noexcept = 0;
		virtual long WriteFromCallback(Utilz::GUID guid, Utilz::GUID type, uint64_t size, const std::function<bool(std::ostream* file)>& function)noexcept = 0;

		virtual long Destroy(Utilz::GUID guid, Utilz::GUID type) noexcept = 0;

		virtual long Defrag()noexcept = 0;

		virtual long GetFilesOfType(Utilz::GUID type, std::vector<File>& files) const noexcept = 0;
		virtual long GetFilesOfType(Utilz::GUID type, FILE_C files[], uint32_t numFiles) const noexcept = 0;
		virtual long GetFiles(std::vector<File>& files)const noexcept = 0;
		virtual long GetFiles(FILE_C* files, uint32_t numfiles)const noexcept = 0;
		virtual long GetFile(FILE_C& files, Utilz::GUID guid, Utilz::GUID type)const noexcept = 0;
		virtual long GetSizeOfFile(Utilz::GUID guid, Utilz::GUID type, uint64_t& size)const noexcept = 0;
		virtual uint32_t GetNumberOfFiles()const noexcept = 0;
		virtual uint32_t GetNumberOfTypes()const noexcept = 0;
		virtual uint32_t GetNumberOfFilesOfType(Utilz::GUID type)const noexcept = 0;
		virtual uint64_t GetTotalSizeOfAllFiles()const noexcept = 0;
		virtual float GetFragmentationRatio()const noexcept = 0;
	protected:
		Loader_Interface() {};
	};

	enum class LoaderType : uint32_t
	{
		Binary
	};

}

DECLDIR_RH_C ResourceHandler::Loader_Interface* CreateLoader(ResourceHandler::LoaderType);
DECLDIR_RH_C long DestroyLoader(ResourceHandler::Loader_Interface*);
DECLDIR_RH_C long InitLoader_C(ResourceHandler::Loader_Interface*,const char* filePath, ResourceHandler::Mode);
DECLDIR_RH_C long Read_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type, void* data, uint64_t size);
DECLDIR_RH_C long Destroy_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type);
DECLDIR_RH_C long CreateS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, void* data, uint64_t size);
DECLDIR_RH_C long CreateFromFile_C(ResourceHandler::Loader_Interface*l, const char* path, const char* guid, const char* type);
DECLDIR_RH_C long Exist_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type);
DECLDIR_RH_C long ExistS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type);
DECLDIR_RH_C long ReadS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, void* data, uint64_t size);
DECLDIR_RH_C long WriteS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, void* data, uint64_t size);
DECLDIR_RH_C long DestroyS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type);
DECLDIR_RH_C long Defrag_C(ResourceHandler::Loader_Interface*);
DECLDIR_RH_C long GetSizeOfFile_C(ResourceHandler::Loader_Interface*, uint32_t guid, uint32_t type, uint64_t* size);
DECLDIR_RH_C long GetSizeOfFileS_C(ResourceHandler::Loader_Interface*, const char* guid, const char* type, uint64_t* size);
DECLDIR_RH_C uint32_t GetNumberOfFiles_C(ResourceHandler::Loader_Interface*);
DECLDIR_RH_C uint32_t GetNumberOfTypes_C(ResourceHandler::Loader_Interface*);
DECLDIR_RH_C uint64_t GetTotalSizeOfAllFiles_C(ResourceHandler::Loader_Interface*);
DECLDIR_RH_C float GetFragmentationRatio_C(ResourceHandler::Loader_Interface*);



DECLDIR_RH_C long GetFiles_C(ResourceHandler::Loader_Interface* l,
	ResourceHandler::FILE_C* files,
	uint32_t  numFiles);

DECLDIR_RH_C long GetFile_C(ResourceHandler::Loader_Interface* l,
	ResourceHandler::FILE_C* file,
	uint32_t guid,
	uint32_t type);
DECLDIR_RH_C uint32_t GetNumberOfFilesOfType_C(ResourceHandler::Loader_Interface*l, const char* type);
DECLDIR_RH_C long GetFilesOfType_C(ResourceHandler::Loader_Interface* l, const char* type, ResourceHandler::FILE_C* files, uint32_t numFiles);
#endif
