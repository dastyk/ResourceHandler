#ifndef _UTILZ_PATH_PARSING_H_
#define _UTILZ_PATH_PARSING_H_

#include <string>
#include <Windows.h>
#include <vector>
#include <codecvt>

	namespace Utilz
	{
		inline std::string getExtension(const std::string& path)
		{
			const size_t period_idx = path.find_last_of('.');
			if (std::string::npos != period_idx)
			{
				return path.substr(period_idx + 1);
			}
			return path;
		}
		inline std::string removeExtension(const std::string& path)
		{
			const size_t period_idx = path.find_last_of('.');
			if (std::string::npos != period_idx)
			{
				return path.substr(0, period_idx);
			}
			return path;
		}

		inline std::string getPath(const std::string& path)
		{
			const size_t last_slash_idx = path.find_last_of('\\/');
			if (std::string::npos != last_slash_idx)
			{
				return path.substr(0, last_slash_idx);
			}
			return path;
		}

		inline std::string getFilename(std::string const& path)
		{
			const size_t last_slash_idx = path.find_last_of("\\/");
			if (std::string::npos != last_slash_idx)
			{
				return path.substr(last_slash_idx + 1);
			}
			return path;
		}

		inline std::string removeRoot(const std::string& path)
		{
			const size_t first_slash_idx = path.find_first_of('\\/');
			if (std::string::npos != first_slash_idx)
			{
				return path.substr(first_slash_idx + 1);
			}
			return path;
		}
		struct File
		{
			std::string name;
			std::string fullPath;
		};

		inline void get_all_files_names_within_folder(std::string folder, std::vector<File>& files)
		{
			std::string search_path = folder + "/*";
			WIN32_FIND_DATA fd;
			HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (fd.cFileName)
					{
						std::string name = fd.cFileName;
						if (!(name == "." || name == ".."))
						{
							if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
								files.push_back({ name, folder + "/" + name });
							else
								get_all_files_names_within_folder(folder + "/" + name, files);
						}
					}

				} while (::FindNextFile(hFind, &fd));
				::FindClose(hFind);
			}

		}

		inline std::wstring utf8ToUtf16(const std::string& utf8Str)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			return conv.from_bytes(utf8Str);
		}

		inline std::string utf16ToUtf8(const std::wstring& utf16Str)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			return conv.to_bytes(utf16Str);
		}
}

#endif _UTILZ_PATH_PARSING_H_