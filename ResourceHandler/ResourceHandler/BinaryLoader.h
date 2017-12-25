#ifndef _RESOURCE_HANDLER_BINARY_LOADER_H_
#define _RESOURCE_HANDLER_BINARY_LOADER_H_
#include <unordered_map>
#include <fstream>

#include "Loader_Interface.h"

namespace ResourceHandler
{
	class BinaryLoader : public Loader_Interface
	{
	public:
		BinaryLoader()noexcept;
		~BinaryLoader()noexcept;

		long Init(const char* filePath, Mode mode)noexcept override;
		long Shutdown()noexcept override;

		long FindType(Utilz::GUID guid, Utilz::GUID& type)const noexcept override;
		long FindNameAndType(Utilz::GUID guid, Utilz::GUID& name, Utilz::GUID& type)const noexcept override;
		long Exist(Utilz::GUID guid, Utilz::GUID type)const noexcept override;
		long Read(Utilz::GUID guid, Utilz::GUID type,const ResourceData& data) noexcept override;

		long Create(Utilz::GUID, Utilz::GUID type, const ResourceData& data)noexcept override;
		long Destroy(Utilz::GUID, Utilz::GUID type)noexcept override;

		long Defrag()noexcept override;

		long GetFilesOfType(Utilz::GUID type, std::vector<File>& files) const noexcept override;
		long GetFiles(std::vector<File>& files)const noexcept override;
		long GetSizeOfFile(Utilz::GUID guid, Utilz::GUID type, uint64_t& size)const noexcept override;
		uint32_t GetNumberOfFiles()const noexcept override;
		uint32_t GetNumberOfTypes()const noexcept override;
		uint64_t GetTotalSizeOfAllFiles()const noexcept override;
	private:
		const char* filePath;
		std::fstream file;
		Mode mode;
		struct FileHeader
		{
			uint32_t tailSize;
			uint64_t endOfFiles;
			uint32_t numFiles;
		}fileHeader;
		
		void AddFile(uint64_t size, void* data);
		void RemoveFile(uint32_t index);
		void ReadTail();

		struct FileEntries
		{
			std::vector <Utilz::GUID>	guid;
			std::vector <Utilz::GUID>	type;
			std::vector <uint64_t>		rawSize;
			std::vector <uint64_t>		size;
			std::vector <uint64_t>		location;
		}entries;
		std::vector<std::unordered_map<Utilz::GUID, uint32_t, Utilz::GUID::Hasher>> typeIndexToFiles;
		std::unordered_map<Utilz::GUID, uint32_t, Utilz::GUID::Hasher> typeToIndex;
	};
}
#endif