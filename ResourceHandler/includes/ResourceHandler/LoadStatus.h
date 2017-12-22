#ifndef _RESOURCE_HANDLER_LOAD_STATUS_H_
#define _RESOURCE_HANDLER_LOAD_STATUS_H_
#include <stdint.h>
#include <Utilz\Flags.h>

namespace ResourceHandler
{
	enum class LoadStatus
	{
		SUCCESS				=		1 << 0,
		NOT_FOUND			=		1 << 1,
		PASS_THROUGH_FAILED	=		1 << 2,
		FAILED				=		1 << 3,
		COULD_NOT_LOAD		=		1 << 4,
		LOADED				=		1 << 5,
		NOT_LOADED			=		1 << 6,
		NOT_CHECKED_IN		=		1 << 7,
		LOADING				=		1 << 8
	};


}
ENUM_FLAGS(ResourceHandler::LoadStatus);


#endif // _RESOURCE_HANDLER_LOAD_STATUS_H_
