#ifndef _RESOURCE_HANDLER_INTERFACE_H_
#define _RESOURCE_HANDLER_INTERFACE_H_
#include <memory>
#include <functional>

#include <GUID.h>
//#include <Utilz\ThreadPool.h>

#include "Resource.h"
#include "FileSystem_Interface.h"
#include "../DLLExport.h"
namespace std
{
	template<class T>
	inline std::unique_ptr<T> make_unique(T* ptr)
	{
		return std::unique_ptr<T>(ptr);
	}
	template<class T>
	inline std::unique_ptr<T> make_unique(T* ptr, std::function<void(T*)>&& deleter)
	{
		return std::unique_ptr<T, std::function<void(T*)>>(ptr, deleter);
	}
}
namespace Utilities
{
	class ThreadPool;
}
namespace ResourceHandler
{

	typedef Utilities::Error(__cdecl *Passthrough_Parse_PROC)(uint32_t guid, void* data, uint64_t size, void** parsedData, uint64_t* parsedSize);
	typedef Utilities::Error(__cdecl *Passthrough_DestroyParsedData_PROC)(uint32_t guid, void* data, uint64_t size);
	typedef Utilities::Error(__cdecl *Passthrough_Load_PROC)(uint32_t guid, void* data, uint64_t size, void** loadedDataRAM, uint64_t* loadedSizeRAM, uint64_t* loadedSizeVRAM);
	typedef Utilities::Error(__cdecl *Passthrough_Unload_PROC)(uint32_t guid, void* dataRAM, uint64_t sizeRAM, uint64_t sizeVRAM);

	using Passthrough_Parse_Func = std::function<Utilities::Error(uint32_t guid, void* data, uint64_t size, void** parsedData, uint64_t* parsedSize)>;
	using Passthrough_DestroyParsedData_Func = std::function<Utilities::Error(uint32_t guid, void* data, uint64_t size)>;
	using Passthrough_Load_Func = std::function<Utilities::Error(uint32_t guid, void* data, uint64_t size, void** loadedDataRAM, uint64_t* loadedSizeRAM, uint64_t* loadedSizeVRAM)>;
	using Passthrough_Unload_Func = std::function<Utilities::Error(uint32_t guid, void* dataRAM, uint64_t sizeRAM, uint64_t sizeVRAM)>;
	
	//struct Passthrough_Info
	//{
	//	Passthrough_Parse_PROC Parse = nullptr;
	//	Passthrough_Destroy_PROC Destroy = nullptr;
	//};
	struct Passthrough_Info
	{
		Passthrough_Parse_Func Parse;
		Passthrough_DestroyParsedData_Func DestroyParsedData;
		Passthrough_Load_Func Load;
		Passthrough_Unload_Func Unload;		
	};
	struct Type_Info
	{
		Passthrough_Info passthrough;
	};
	struct Passthrough_LoadInfo
	{
		uint64_t librarySize = 0;
		char* library = nullptr;
	};
	struct Type_LoadInfo
	{
		Passthrough_LoadInfo passthrough;
	};
	class ResourceHandler_Interface
	{
		friend class Resource;
	public:
	

		virtual ~ResourceHandler_Interface() {};
		virtual UERROR CreateType(const std::string& type, const Type_LoadInfo& info, bool force = false) = 0;
		virtual UERROR AddType(Utilities::GUID type, const Type_Info& info) = 0;
		virtual UERROR Initialize() = 0;
		virtual void Shutdown() = 0;
		
	protected:
		ResourceHandler_Interface() {};

		virtual	void LoadResource(const Resource& resource, bool invalid = false) = 0;
		virtual LoadStatus GetData(const Resource& resource, ResourceDataVoid& data) = 0;
		virtual LoadStatus PeekStatus(const Resource& resource)const = 0;
		virtual void CheckIn(const Resource& resource) = 0;
		virtual void CheckOut(const Resource& resource) = 0;
		virtual uint32_t GetReferenceCount(const Resource& resource)const = 0;
		virtual void Invalidate(const Resource& resource) = 0;
	};

	DECLDIR_RH ResourceHandler_Interface* Get();

}
DECLDIR_RH_C void DestroyThreadPool(Utilities::ThreadPool* tp);
DECLDIR_RH_C void DestroyResourceHandler(ResourceHandler::ResourceHandler_Interface* rh);
DECLDIR_RH_C Utilities::ThreadPool* CreateThreadPool(uint32_t numThreads);
DECLDIR_RH_C ResourceHandler::ResourceHandler_Interface* CreateResourceHandler(ResourceHandler::FileSystem_Interface* loader, Utilities::ThreadPool* threadPool);
DECLDIR_RH_C  Utilities::Error ResourceHandler_CreateType(ResourceHandler::ResourceHandler_Interface* rh, const char* type, const char* passthrough);
#endif // _RESOURCE_HANDLER_INTERFACE_H_