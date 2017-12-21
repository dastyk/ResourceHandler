#ifndef _RESOURCE_HANDLER_INTERFACE_H_
#define _RESOURCE_HANDLER_INTERFACE_H_
#include <memory>

#include <GUID.h>

#include "Resource.h"
namespace std
{
	template<class T>
	inline std::unique_ptr<T> make_unique(T* ptr)
	{
		return std::unique_ptr<T>(ptr);
	}
	template<class T, class LAMBDA>
	inline std::unique_ptr<T> make_unique(T* ptr, const LAMBDA& l)
	{
		return std::unique_ptr<T>(ptr, l);
	}
}

namespace ResourceHandler
{
	class ResourceHandler_Interface
	{
		friend class Resource;
	public:
		virtual ~ResourceHandler_Interface() {};

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
	DECLDIR ResourceHandler_Interface* CreateResourceHandler();
}

#endif // _RESOURCE_HANDLER_INTERFACE_H_