#include <stdint.h>

extern "C" __declspec(dllexport) int32_t Parse(uint32_t guid, void* data, uint64_t size, void** parsedData, uint64_t* parsedSize)
{
	uint32_t& num = *(uint32_t*)data;
	*parsedSize = size;
	*parsedData = new uint32_t(num + 1);
	return 0;
}
extern "C" __declspec(dllexport) int32_t Destroy(uint32_t guid, void* data, uint64_t size)
{
	delete (uint32_t*)data;
	return 0; 
}
