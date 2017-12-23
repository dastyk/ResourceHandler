#ifndef _RESOURCE_HANDLER_LOADER_INTERFACE_H_
#define _RESOURCE_HANDLER_LOADER_INTERFACE_H_
#include <GUID.h>

#include "ResourceData.h"

namespace ResourceHandler
{
	enum class Mode
	{
		EDIT,
		READ
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
		virtual long Read(Utilz::GUID guid, Utilz::GUID type, ResourceData& data) noexcept = 0;

		virtual long Create(Utilz::GUID, Utilz::GUID type, const ResourceData& data) noexcept = 0;
		virtual long Destroy(Utilz::GUID, Utilz::GUID type) noexcept = 0;

		virtual long Defrag()noexcept = 0;

		virtual size_t GetNumberOfFiles()const noexcept = 0;
		virtual size_t GetNumberOfTypes()const noexcept = 0;
		virtual size_t GetTotalSizeOfAllFiles()const noexcept = 0;
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
extern "C" DECLDIR long InitLoader(ResourceHandler::Loader_Interface*,const char* filePath, ResourceHandler::Mode);

#endif
