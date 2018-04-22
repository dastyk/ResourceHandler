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
	inline operator const ResourceDataVoid& ()const { return data; }
	inline ResourceDataVoid& GetVoid()
	{
		return data;
	}
	inline const T& Get()const
	{
		return *(T*)data.data;
	}
	const T* operator->()const
	{
		return (T*)data.data;
	}
	const T& operator*()const
	{
		return *(T*)data.data;
	}
	const char* GetExtra()const
	{
		return (char*)data.data + sizeof(T) - sizeof(char*);
	}
	operator ResourceDataVoid&() { return data; }
private:
	ResourceDataVoid data;
};
template<>
struct ResourceData<char*>
{
	operator ResourceDataVoid&() { return data; }
	inline ResourceDataVoid& GetVoid()
	{
		return data;
	}
	inline char* Get()const
	{
		return (char*)data.data;
	}
private:
	ResourceDataVoid data;
};



#endif