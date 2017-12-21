#ifndef _RESOURCE_HANDLER_BINARY_LOADER_H_
#define _RESOURCE_HANDLER_BINARY_LOADER_H_
#include <unordered_map>
#include <fstream>


#include "Loader_Interface.h"
#include "Sofa.h"

namespace ResourceHandler
{
	class BinaryLoader : public Loader_Interface
	{
	public:
		BinaryLoader()noexcept;
		~BinaryLoader()noexcept;

		long Init(Mode mode)noexcept override;
		long Shutdown()noexcept override;

		long Exist(Utilz::GUID guid, Utilz::GUID type)const noexcept override;
		long Read(Utilz::GUID guid, Utilz::GUID type, ResourceData& data) noexcept override;

		long Create(Utilz::GUID, Utilz::GUID type, const ResourceData& data)noexcept override;
		long Destroy(Utilz::GUID, Utilz::GUID type)noexcept override;

		size_t GetNumberOfFiles()const noexcept override;
		size_t GetNumberOfTypes()const noexcept override;
	private:
		std::fstream file;
		Mode mode;
		struct FileHeader
		{
			size_t tailSize;
			size_t endOfFiles;
			size_t numFiles;
		}fileHeader;
		
		void AddFile(size_t size, void* data);
		void RemoveFile(size_t index);
		void WriteTail();
		void ReadTail();

		struct FileEntries
		{
			std::vector <Utilz::GUID>	guid;
			std::vector <Utilz::GUID>	type;
			std::vector <size_t>		rawSize;
			std::vector <size_t>		size;
			std::vector <size_t>		location;
		}entries;
		std::vector<std::unordered_map<Utilz::GUID, size_t, Utilz::GUID::Hasher>> typeIndexToFiles;
		std::unordered_map<Utilz::GUID, size_t, Utilz::GUID::Hasher> typeToIndex;
	};
}
#endif