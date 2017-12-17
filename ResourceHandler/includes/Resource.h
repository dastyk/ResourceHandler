#ifndef _RESOURCE_HANDLER_RESOURCE_H_
#define _RESOURCE_HANDLER_RESOURCE_H_

#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
#include <future>
namespace ResourceHandler
{
	DECLDIR class Resource
	{
	public:
		Resource();
		~Resource();

		void Unload();
	private:
		std::future<LoadStatus>
	};
}


#endif // _RESOURCE_HANDLER_RESOURCE_H_