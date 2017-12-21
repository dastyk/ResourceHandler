#include "DirectoryLoader.h"


namespace ResourceHandler
{
	DirectoryLoader::DirectoryLoader(Mode mode)noexcept
	{
		switch (mode)
		{
		case ResourceHandler::Mode::EDIT:
			break;
		case ResourceHandler::Mode::READ:
			break;
		default:
			break;
		}
	}


	DirectoryLoader::~DirectoryLoader()noexcept
	{
	}
	long DirectoryLoader::Exist(Utilz::GUID guid, Utilz::GUID type)const noexcept
	{
		return 0;
	}
	long DirectoryLoader::Read(Utilz::GUID guid, Utilz::GUID type, ResourceData & data) noexcept
	{
		return 0;
	}
	long DirectoryLoader::Create(Utilz::GUID, Utilz::GUID type, const ResourceData & data)noexcept
	{
		return 0;
	}
	long DirectoryLoader::Destroy(Utilz::GUID, Utilz::GUID type)noexcept
	{
		return 0;
	}
}