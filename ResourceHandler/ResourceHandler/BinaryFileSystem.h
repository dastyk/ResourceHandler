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

		UERROR Init(const char* filePath, Mode mode)noexcept override;
		UERROR Shutdown()noexcept override;

		UERROR FindType(Utilities::GUID guid, Utilities::GUID& type)const noexcept override;
		UERROR FindNameAndType(Utilities::GUID guid, Utilities::GUID& name, Utilities::GUID& type)const noexcept override;
		bool       Exist(Utilities::GUID guid, Utilities::GUID type)const noexcept override;
		UERROR Read(Utilities::GUID guid, Utilities::GUID type,const ResourceDataVoid& data) noexcept override;

		UERROR Create(const std::string&, const std::string& type, const ResourceDataVoid& data)noexcept override;
		UERROR CreateFromFile(const char* filePath, const std::string& guid, const std::string& type) noexcept override;
		UERROR CreateFromCallback(const std::string& guid, const std::string& type, const std::function<bool(std::ostream* file)>& function)noexcept override;
		
		UERROR Write(Utilities::GUID guid, Utilities::GUID type, const ResourceDataVoid& data)noexcept override;
		UERROR WriteFromCallback(Utilities::GUID guid, Utilities::GUID type, uint64_t size, const std::function<bool(std::ostream* file)>& function)noexcept override;

		UERROR Destroy(Utilities::GUID, Utilities::GUID type)noexcept override;

		UERROR Defrag()noexcept override;

		UERROR GetFilesOfType(Utilities::GUID type, std::vector<File>& files) const noexcept override;
		UERROR GetFilesOfType(Utilities::GUID type, FILE_C files[], uint32_t numFiles) const noexcept override;
		UERROR GetFiles(std::vector<File>& files)const noexcept override;
		UERROR GetFiles(FILE_C* files, uint32_t numfiles)const noexcept override;
		UERROR GetFile(FILE_C& files, Utilities::GUID guid, Utilities::GUID type)const noexcept override;
		UERROR GetSizeOfFile(Utilities::GUID guid, Utilities::GUID type, uint64_t& size)const noexcept override;
		uint32_t GetNumberOfFiles()const noexcept override;
		uint32_t GetNumberOfTypes()const noexcept override;
		uint32_t GetNumberOfFilesOfType(Utilities::GUID type)const noexcept override;
		uint64_t GetTotalSizeOfAllFiles()const noexcept override;
		float GetFragmentationRatio()const noexcept override;

	private:
		mutable std::recursive_mutex lock;
		std::string filePath;
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
			std::vector <Utilities::GUID>	guid;
			std::vector <Utilities::GUID>	type;
			std::vector <uint64_t>		rawSize;
			std::vector <uint64_t>		size;
			std::vector <uint64_t>		location;
			std::vector <std::string>	guid_str;
			std::vector <std::string>	type_str;
		}entries;
		std::vector<std::unordered_map<Utilities::GUID, uint32_t, Utilities::GUID::Hasher>> typeIndexToFiles;
		std::unordered_map<Utilities::GUID, uint32_t, Utilities::GUID::Hasher> typeToIndex;
	};
}
#endif