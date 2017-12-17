#ifndef _RESOURCE_HANDLER_LOAD_STATUS_H_
#define _RESOURCE_HANDLER_LOAD_STATUS_H_
#include <stdint.h>

namespace ResourceHandler
{
	enum class LoadStatus : uint32_t
	{
		SUCCESS				=		1 << 0,
		NOT_FOUND			=		1 << 1,
		COULD_NOT_PARSE		=		1 << 2,
		FAILED				=		1 << 3
	};
}



#endif // _RESOURCE_HANDLER_LOAD_STATUS_H_
