#ifndef _UTILZ_STRING_READ_WRITE_H_
#define _UTILZ_STRING_READ_WRITE_H_
#include <string>

namespace Utilz
{
	void writeString(std::ostream* out, const std::string& str)
	{
		uint32_t size = static_cast<uint32_t>(str.size());
		out->write((char*)&size, sizeof(size));
		out->write(str.c_str(), size);
	}
	void readString(std::istream* in, std::string& str)
	{
		uint32_t size = 0;
		in->read((char*)&size, sizeof(size));
		char buffer[512];
		in->read(buffer, size);
		str = std::string(buffer, size);
	}
}

#endif