#ifndef _RESOURCE_HANDLER_RESOURCE_DATA_H_
#define _RESOURCE_HANDLER_RESOURCE_DATA_H_

struct ResourceDataVoid
{
	void* data = nullptr;
	uint64_t size = 0;
};

template<class T>
struct ResourceData
{
	ResourceDataVoid& GetVoid()
	{
		return data;
	}
	const T& Get()const
	{
		return *(T*)data.data;
	}
private:
	ResourceDataVoid data;
};



#endif