#ifndef _RESOURCE_HANDLER_RESOURCE_DATA_H_
#define _RESOURCE_HANDLER_RESOURCE_DATA_H_
#include "Resource.h"
#include <Error.h>

namespace ResourceHandler
{


	template<class T>
	struct ResourceData : public Resource
	{
		ResourceData(Utilities::GUID guid, Utilities::GUID type) : Resource(guid, type)
		{

		}
		ResourceData(Resource resource) : Resource(resource)
		{
		}
		inline operator const T& () { return Get(); }
		inline const T& Get()
		{
			if (!(status & LoadStatus::LOADED))
			{
				status = GetData(data);
				if (!(status & LoadStatus::LOADED))
					THROW_ERROR_EX("Could not get data for resource", GUID());
			}
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
		/*const char* GetExtra()
		{
			return ((char*)&Get()) + sizeof(T) - sizeof(char*);
		}*/

	private:
		LoadStatus status;
		ResourceDataVoid data;
	};
	template<>
	struct ResourceData<char*> : public Resource
	{
		ResourceData(Utilities::GUID guid, Utilities::GUID type) : Resource(guid, type)
		{

		}
		ResourceData(Resource resource) : Resource(resource)
		{

		}
		operator char*() { return Get(); }
		inline char* Get()
		{
			if (!(status & LoadStatus::LOADED))
			{
				status = GetData(data);
				if (!(status & LoadStatus::LOADED))
					THROW_ERROR_EX("Could not get data for resource", GUID());
			}
			return (char*)data.data;
		}
		const size_t GetSize()
		{
			Get();
			return data.size;
		}
	private:
		LoadStatus status;
		ResourceDataVoid data;
	};

}

#endif