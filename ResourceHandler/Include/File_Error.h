#ifndef _RESOURCE_HANDLER_FILE_ERROR_H_
#define _RESOURCE_HANDLER_FILE_ERROR_H_

#include <stdint.h>


#define RETURN_FILE_ERROR(msg, nr) return (ResourceHandler::_lastError = ResourceHandler::File_Error{msg, nr, __FILE__, __LINE__})
#define RETURN_FILE_ERROR_C(msg) return (ResourceHandler::_lastError = ResourceHandler::File_Error{msg, -(__COUNTER__+1), __FILE__, __LINE__})
#define RETURN_IF_FILE_ERROR(result, msg) if((result) < 0) return (ResourceHandler::_lastError = ResourceHandler::File_Error{msg, result, __FILE__, __LINE__})
#define PASS_IF_FILE_ERROR(x) if(x.errornr < 0) return x
#define RETURN_FILE_SUCCESS return (ResourceHandler::_lastError = ResourceHandler::File_Error{nullptr, 0, nullptr, 0})
#define FILE_ERROR const ResourceHandler::File_Error&
namespace ResourceHandler
{
	struct File_Error
	{
		const char* errorMSG;	
		int32_t errornr;
		const char* file;
		int32_t line;
	};
	static File_Error _lastError;
}
#endif