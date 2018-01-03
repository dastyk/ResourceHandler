#ifndef _RESOURCE_HANDLER_INTERFACE_H_
#define _RESOURCE_HANDLER_INTERFACE_H_
#include <memory>
#include <functional>

#include <GUID.h>
#include <Utilz\ThreadPool.h>

#include "Resource.h"
#include "Loader_Interface.h"
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

namespace ResourceHandler
{
	enum class MemoryType
	{
		RAM,
		VRAM
	};

	class ResourceHandler_Interface
	{
		friend class Resource;
	public:
		using PassThroughCallback = std::function<void()>;

		virtual ~ResourceHandler_Interface() {};
		virtual long CreateTypePassthrough(Utilz::GUID type, MemoryType memoryType, const PassThroughCallback& passThrough) = 0;
	
	protected:
		ResourceHandler_Interface() {};

		virtual	void LoadResource(const Resource& resource) = 0;
		virtual LoadStatus GetData(const Resource& resource, ResourceDataVoid& data) = 0;
		virtual LoadStatus PeekStatus(const Resource& resource)const = 0;
		virtual void CheckIn(const Resource& resource) = 0;
		virtual void CheckOut(const Resource& resource) = 0;
		virtual uint32_t GetReferenceCount(const Resource& resource)const = 0;
		virtual void Invalidate(const Resource& resource) = 0;
	};

	
}
DECLDIR_RH_C void DestroyThreadPool(Utilz::ThreadPool* tp);
DECLDIR_RH_C void DestroyResourceHandler(ResourceHandler::ResourceHandler_Interface* rh);
DECLDIR_RH_C Utilz::ThreadPool* CreateThreadPool(uint32_t numThreads);
DECLDIR_RH_C ResourceHandler::ResourceHandler_Interface* CreateResourceHandler(ResourceHandler::Loader_Interface* loader, Utilz::ThreadPool* threadPool);
#endif // _RESOURCE_HANDLER_INTERFACE_H_