#ifndef _RESOURCE_HANDLER_INTERFACE_H_
#define _RESOURCE_HANDLER_INTERFACE_H_
#include <memory>
#include <functional>

#include <GUID.h>
#include <Utilz\ThreadPool.h>

#include "Resource.h"
#include "Loader_Interface.h"

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
		virtual	Resource LoadResource(Utilz::GUID guid, Utilz::GUID type) = 0;
	
	protected:
		virtual const ResourceData GetData(Utilz::GUID guid)const = 0;
		virtual LoadStatus GetStatus(Utilz::GUID guid) = 0;

		ResourceHandler_Interface() {};

	};


#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
	DECLDIR ResourceHandler_Interface* CreateResourceHandler(Loader_Interface* loader, Utilz::ThreadPool* threadPool);
}

#endif // _RESOURCE_HANDLER_INTERFACE_H_