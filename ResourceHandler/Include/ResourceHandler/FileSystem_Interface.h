#ifndef _RESOURCE_HANDLER_FileSystem_Interface_H_
#define _RESOURCE_HANDLER_FileSystem_Interface_H_
#include <string>
#include <vector>
#include <functional>

#include <GUID.h>

#include "ResourceData.h"
#include "../DLLExport.h"
#include <File_Error.h>
namespace ResourceHandler
{
	enum class FileSystemType : uint32_t
	{
		Binary
	};

	enum class Mode
	{
		EDIT,
		READ
	};

	struct File
	{
		Utilities::GUID guid;
		Utilities::GUID type;
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
	class FileSystem_Interface
	{
	public:
		virtual ~FileSystem_Interface() {};
		virtual FILE_ERROR Init(const char* filePath, Mode mode) noexcept = 0;
		virtual FILE_ERROR Shutdown() noexcept = 0; 

		virtual FILE_ERROR FindType(Utilities::GUID guid, Utilities::GUID& type)const noexcept = 0;
		virtual FILE_ERROR FindNameAndType(Utilities::GUID guid, Utilities::GUID& name, Utilities::GUID& type)const noexcept = 0;
		virtual bool	   Exist(Utilities::GUID guid, Utilities::GUID type)const noexcept = 0;
		virtual FILE_ERROR Read(Utilities::GUID guid, Utilities::GUID type, const ResourceDataVoid& data) noexcept = 0;
			
		virtual FILE_ERROR Create(const std::string& guid,const std::string& type, const ResourceDataVoid& data) noexcept = 0;
		virtual FILE_ERROR CreateFromFile(const char* filePath, const std::string& guid, const std::string& type) noexcept = 0;
		virtual FILE_ERROR CreateFromCallback(const std::string& guid, const std::string& type, const std::function<bool(std::ostream* file)>& function)noexcept = 0;
			
		virtual FILE_ERROR Write(Utilities::GUID guid, Utilities::GUID type, const ResourceDataVoid& data)noexcept = 0;
		virtual FILE_ERROR WriteFromCallback(Utilities::GUID guid, Utilities::GUID type, uint64_t size, const std::function<bool(std::ostream* file)>& function)noexcept = 0;
			
		virtual FILE_ERROR Destroy(Utilities::GUID guid, Utilities::GUID type) noexcept = 0;
		
		virtual FILE_ERROR Defrag()noexcept = 0;

		virtual FILE_ERROR GetFilesOfType(Utilities::GUID type, std::vector<File>& files) const noexcept = 0;
		virtual FILE_ERROR GetFilesOfType(Utilities::GUID type, FILE_C files[], uint32_t numFiles) const noexcept = 0;
		virtual FILE_ERROR GetFiles(std::vector<File>& files)const noexcept = 0;
		virtual FILE_ERROR GetFiles(FILE_C* files, uint32_t numfiles)const noexcept = 0;
		virtual FILE_ERROR GetFile(FILE_C& files, Utilities::GUID guid, Utilities::GUID type)const noexcept = 0;
		virtual FILE_ERROR GetSizeOfFile(Utilities::GUID guid, Utilities::GUID type, uint64_t& size)const noexcept = 0;
		virtual uint32_t GetNumberOfFiles()const noexcept = 0;
		virtual uint32_t GetNumberOfTypes()const noexcept = 0;
		virtual uint32_t GetNumberOfFilesOfType(Utilities::GUID type)const noexcept = 0;
		virtual uint64_t GetTotalSizeOfAllFiles()const noexcept = 0;
		virtual float GetFragmentationRatio()const noexcept = 0;
	protected:
		FileSystem_Interface() {};
	};


}

DECLDIR_RH_C ResourceHandler::FileSystem_Interface* CreateFileSystem(ResourceHandler::FileSystemType);
DECLDIR_RH_C ResourceHandler::File_Error DestroyLoader(ResourceHandler::FileSystem_Interface*);
DECLDIR_RH_C ResourceHandler::File_Error InitLoader_C(ResourceHandler::FileSystem_Interface*,const char* filePath, ResourceHandler::Mode);
DECLDIR_RH_C ResourceHandler::File_Error Read_C(ResourceHandler::FileSystem_Interface*, uint32_t guid, uint32_t type, void* data, uint64_t size);
DECLDIR_RH_C ResourceHandler::File_Error Destroy_C(ResourceHandler::FileSystem_Interface*, uint32_t guid, uint32_t type);
DECLDIR_RH_C ResourceHandler::File_Error CreateS_C(ResourceHandler::FileSystem_Interface*, const char* guid, const char* type, void* data, uint64_t size);
DECLDIR_RH_C ResourceHandler::File_Error CreateFromFile_C(ResourceHandler::FileSystem_Interface*l, const char* path, const char* guid, const char* type);
DECLDIR_RH_C bool						 Exist_C(ResourceHandler::FileSystem_Interface*, uint32_t guid, uint32_t type);
DECLDIR_RH_C bool						 ExistS_C(ResourceHandler::FileSystem_Interface*, const char* guid, const char* type);
DECLDIR_RH_C ResourceHandler::File_Error ReadS_C(ResourceHandler::FileSystem_Interface*, const char* guid, const char* type, void* data, uint64_t size);
DECLDIR_RH_C ResourceHandler::File_Error WriteS_C(ResourceHandler::FileSystem_Interface*, const char* guid, const char* type, void* data, uint64_t size);
DECLDIR_RH_C ResourceHandler::File_Error DestroyS_C(ResourceHandler::FileSystem_Interface*, const char* guid, const char* type);
DECLDIR_RH_C ResourceHandler::File_Error Defrag_C(ResourceHandler::FileSystem_Interface*);
DECLDIR_RH_C ResourceHandler::File_Error GetSizeOfFile_C(ResourceHandler::FileSystem_Interface*, uint32_t guid, uint32_t type, uint64_t* size);
DECLDIR_RH_C ResourceHandler::File_Error GetSizeOfFileS_C(ResourceHandler::FileSystem_Interface*, const char* guid, const char* type, uint64_t* size);
DECLDIR_RH_C uint32_t GetNumberOfFiles_C(ResourceHandler::FileSystem_Interface*);
DECLDIR_RH_C uint32_t GetNumberOfTypes_C(ResourceHandler::FileSystem_Interface*);
DECLDIR_RH_C uint64_t GetTotalSizeOfAllFiles_C(ResourceHandler::FileSystem_Interface*);
DECLDIR_RH_C float GetFragmentationRatio_C(ResourceHandler::FileSystem_Interface*);



DECLDIR_RH_C ResourceHandler::File_Error GetFiles_C(ResourceHandler::FileSystem_Interface* l,
	ResourceHandler::FILE_C* files,
	uint32_t  numFiles);

DECLDIR_RH_C ResourceHandler::File_Error GetFile_C(ResourceHandler::FileSystem_Interface* l,
	ResourceHandler::FILE_C* file,
	const char* guid,
	const char* type);
DECLDIR_RH_C uint32_t GetNumberOfFilesOfType_C(ResourceHandler::FileSystem_Interface*l, const char* type);
DECLDIR_RH_C ResourceHandler::File_Error GetFilesOfType_C(ResourceHandler::FileSystem_Interface* l, const char* type, ResourceHandler::FILE_C* files, uint32_t numFiles);
#endif
