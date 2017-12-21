#ifndef _RESOURCE_HANDLER_DIRECTORY_LOADER_H_
#define _RESOURCE_HANDLER_DIRECTORY_LOADER_H_
#include <unordered_map>
#include <fstream>


#include "Loader_Interface.h"
namespace ResourceHandler
{
	class DirectoryLoader : public Loader_Interface
	{
	public:
		DirectoryLoader(Mode mode)noexcept;
		~DirectoryLoader()noexcept;

		long Exist(Utilz::GUID guid, Utilz::GUID type)const noexcept override;
		long Read(Utilz::GUID guid, Utilz::GUID type, ResourceData& data) noexcept override;
		 
		long Create(Utilz::GUID, Utilz::GUID type, const ResourceData& data)noexcept override;
		long Destroy(Utilz::GUID, Utilz::GUID type)noexcept override;

	private:
		struct FileEntry
		{

		};
		struct TypeEntry
		{
			std::unordered_map<Utilz::GUID, std::string, Utilz::GUID::Hasher> fileTable;
		};
		
		std::unordered_map<Utilz::GUID, TypeEntry, Utilz::GUID::Hasher> typeTable;
	};
}
#endif