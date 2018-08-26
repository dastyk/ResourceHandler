#ifndef _RESOURCE_HANDLER_RESOURCE_DATA_H_
#define _RESOURCE_HANDLER_RESOURCE_DATA_H_
#include "Resource.h"
#include <Error.h>

namespace ResourceHandler
{


	template<class T>
	struct ResourceData
	{
		ResourceData(Utilities::GUID guid, Utilities::GUID type) : resource(guid, type)
		{
			resource.CheckIn();
		}
		ResourceData(Resource resource) : resource(resource)
		{
			resource.CheckIn();
		}
		inline operator const T& () { return Get(); }
		inline const T& Get()
		{
			if (!(status & LoadStatus::LOADED))
				status = resource.GetData(data);
			if (!(status & LoadStatus::LOADED))
				THROW_ERROR_EX("Could not get data for resource", resource.GUID());
			return *(T*)data.data;
		}
		const T* operator->()
		{
			return &Get();
		}
		const T& operator*()
		{
			return Get();
		}
		const size_t GetSize()
		{
			Get();
			return data.size;
		}
		const char* GetExtra()
		{
			return ((char*)&Get()) + sizeof(T) - sizeof(char*);
		}
		inline operator T&() { return Get(); }
	private:
		Resource resource;
		LoadStatus status;
		ResourceDataVoid data;
	};
	template<>
	struct ResourceData<char*>
	{
		ResourceData(Utilities::GUID guid, Utilities::GUID type) : resource(guid, type)
		{
			resource.CheckIn();
		}
		ResourceData(Resource resource) : resource(resource)
		{
			resource.CheckIn();
		}
		operator char*() { return Get(); }
		inline char* Get()
		{
			if (!(status & LoadStatus::LOADED))
				status = resource.GetData(data);
			if (!(status & LoadStatus::LOADED))
				THROW_ERROR_EX("Could not get data for resource", resource.GUID());
			return (char*)data.data;
		}
		const size_t GetSize()
		{
			Get();
			return data.size;
		}
	private:
		
		Resource resource;
		LoadStatus status;
		ResourceDataVoid data;
	};

}

#endif