#include <stdint.h>
#include <Error.h>
extern "C" __declspec(dllexport) Utilities::Error Parse(uint32_t guid, void* data, uint64_t size, void** parsedData, uint64_t* parsedSize)
{
	uint32_t& num = *(uint32_t*)data;
	*parsedSize = size;
	*parsedData = new uint32_t(num + 1);
	RETURN_SUCCESS;
}
extern "C" __declspec(dllexport) Utilities::Error DestroyParsedData(uint32_t guid, void* data, uint64_t size)
{
	delete (uint32_t*)data;
	RETURN_SUCCESS;
}
/*

extern "C" __declspec(dllexport) Utilities::Error Load(uint32_t guid, void* data, uint64_t size, void** loadedDataRAM, uint64_t* loadedSizeRAM, uint64_t* loadedSizeVRAM)
{


	RETURN_SUCCESS;
}
extern "C" __declspec(dllexport) Utilities::Error Unload(uint32_t guid, void* dataRAM, uint64_t sizeRAM, uint64_t sizeVRAM)
{

	RETURN_SUCCESS;
}
*/