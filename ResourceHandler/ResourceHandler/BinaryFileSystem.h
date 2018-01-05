#ifndef _RESOURCE_HANDLER_BINARY_LOADER_H_
#define _RESOURCE_HANDLER_BINARY_LOADER_H_
#include <unordered_map>
#include <fstream>

#include <ResourceHandler\FileSystem_Interface.h>
#include <mutex>

namespace ResourceHandler
{
	class BinaryFileSystem : public FileSystem_Interface
	{
	public:
		static const uint32_t version = 0;

		BinaryFileSystem()noexcept;
		~BinaryFileSystem()noexcept;

		FILE_ERROR Init(const char* filePath, Mode mode)noexcept override;
		FILE_ERROR Shutdown()noexcept override;

		FILE_ERROR FindType(Utilz::GUID guid, Utilz::GUID& type)const noexcept override;
		FILE_ERROR FindNameAndType(Utilz::GUID guid, Utilz::GUID& name, Utilz::GUID& type)const noexcept override;
		bool       Exist(Utilz::GUID guid, Utilz::GUID type)const noexcept override;
		FILE_ERROR Read(Utilz::GUID guid, Utilz::GUID type,const ResourceDataVoid& data) noexcept override;

		FILE_ERROR Create(const std::string&, const std::string& type, const ResourceDataVoid& data)noexcept override;
		FILE_ERROR CreateFromFile(const char* filePath, const std::string& guid, const std::string& type) noexcept override;
		FILE_ERROR CreateFromCallback(const std::string& guid, const std::string& type, const std::function<bool(std::ostream* file)>& function)noexcept override;
		
		FILE_ERROR Write(Utilz::GUID guid, Utilz::GUID type, const ResourceDataVoid& data)noexcept override;
		FILE_ERROR WriteFromCallback(Utilz::GUID guid, Utilz::GUID type, uint64_t size, const std::function<bool(std::ostream* file)>& function)noexcept override;

		FILE_ERROR Destroy(Utilz::GUID, Utilz::GUID type)noexcept override;

		FILE_ERROR Defrag()noexcept override;

		FILE_ERROR GetFilesOfType(Utilz::GUID type, std::vector<File>& files) const noexcept override;
		FILE_ERROR GetFilesOfType(Utilz::GUID type, FILE_C files[], uint32_t numFiles) const noexcept override;
		FILE_ERROR GetFiles(std::vector<File>& files)const noexcept override;
		FILE_ERROR GetFiles(FILE_C* files, uint32_t numfiles)const noexcept override;
		FILE_ERROR GetFile(FILE_C& files, Utilz::GUID guid, Utilz::GUID type)const noexcept override;
		FILE_ERROR GetSizeOfFile(Utilz::GUID guid, Utilz::GUID type, uint64_t& size)const noexcept override;
		uint32_t GetNumberOfFiles()const noexcept override;
		uint32_t GetNumberOfTypes()const noexcept override;
		uint32_t GetNumberOfFilesOfType(Utilz::GUID type)const noexcept override;
		uint64_t GetTotalSizeOfAllFiles()const noexcept override;
		float GetFragmentationRatio()const noexcept override;

	private:
		mutable std::recursive_mutex lock;
		const char* filePath;
		std::fstream file;
		Mode mode;
		struct FileHeader
		{
			uint32_t version;
			uint32_t tailSize;
			uint64_t endOfFiles;
			uint32_t numFiles;
			uint64_t unusedSpace;
		}fileHeader;
		
		void AddFile(uint64_t size, void* data);
		void AddFile(uint64_t size, std::fstream& in);
		void RemoveFile(uint32_t index);
		void ReadTail();

		struct FileEntries
		{
			std::vector <Utilz::GUID>	guid;
			std::vector <Utilz::GUID>	type;
			std::vector <uint64_t>		rawSize;
			std::vector <uint64_t>		size;
			std::vector <uint64_t>		location;
			std::vector <std::string>	guid_str;
			std::vector <std::string>	type_str;
		}entries;
		std::vector<std::unordered_map<Utilz::GUID, uint32_t, Utilz::GUID::Hasher>> typeIndexToFiles;
		std::unordered_map<Utilz::GUID, uint32_t, Utilz::GUID::Hasher> typeToIndex;
	};
}
#endif