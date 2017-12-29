#include "BinaryLoader.h"
#include <Profiler.h>
#include <optional>
#include <filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem;
#ifdef _WIN32
#include <Windows.h>
#undef min
#undef CopyFile
#endif

namespace ResourceHandler
{
	template<class FILE>
	void writeString(FILE& out, const std::string& str)
	{
		uint32_t size = static_cast<uint32_t>(str.size());
		out.write((char*)&size, sizeof(size));
		out.write(str.c_str(), size);
	}
	template<class FILE>
	void readString(FILE& in, std::string& str)
	{
		uint32_t size = 0;
		in.read((char*)&size, sizeof(size));
		char buffer[512];
		in.read(buffer, size);
		str = std::string(buffer, size);
	}
	template<class FILE, class TAIL>
	void WriteTail(FILE& file, TAIL& entries, uint32_t numFiles)
	{
		StartProfile;
		if (numFiles)
		{
			file.write((char*)entries.guid.data(),			sizeof(entries.guid[0])		* numFiles);
			file.write((char*)entries.type.data(),			sizeof(entries.type[0])		* numFiles);
			file.write((char*)entries.rawSize.data(),		sizeof(entries.rawSize[0])	* numFiles);
			file.write((char*)entries.size.data(),			sizeof(entries.size[0])		* numFiles);
			file.write((char*)entries.location.data(),		sizeof(entries.location[0]) * numFiles);
			for (auto& s : entries.guid_str)
				writeString(file, s);
			for (auto& t : entries.type_str)
				writeString(file, t);
		}
	}
	template<class INFILE, class OUTFILE>
	void CopyFile(INFILE& in, OUTFILE& out, uint64_t readPos, uint64_t writePos, uint64_t size)
	{
		StartProfile;
		uint64_t copied = 0;
		char buff[524288];
		while (copied < size)
		{
			size_t toWrite = std::min(uint64_t(524288), size - copied);
			copied += toWrite;
			in.seekg(readPos);
			in.read(buff, toWrite);
			out.seekp(writePos);
			out.write(buff, toWrite);
			writePos += toWrite;
			readPos += toWrite;
		}

	}
	BinaryLoader::BinaryLoader()noexcept
	{
	}


	BinaryLoader::~BinaryLoader()noexcept
	{
	}

	long BinaryLoader::CreateFromCallback(const std::string & guid, const std::string & type, const std::function<bool(std::ostream* file)>& function)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		size_t index;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			index = findType->second;
			auto& files = typeIndexToFiles[index];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				return 1;
		}
		else
		{
			index = typeIndexToFiles.size();
			typeIndexToFiles.push_back({});
			typeToIndex[type] = static_cast<uint32_t>(index);
		}


		file.seekp(fileHeader.endOfFiles);
		if (!function(&file))
			return -2;
		uint64_t size = static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles;
	

		auto& files = typeIndexToFiles[index];
		files[guid] = static_cast<uint32_t>(entries.guid.size());
		entries.guid.push_back(guid);
		entries.type.push_back(type);
		entries.rawSize.push_back(size);
		entries.size.push_back(size);
		entries.location.push_back(fileHeader.endOfFiles);
		entries.guid_str.push_back(guid);
		entries.type_str.push_back(type);
		fileHeader.endOfFiles = file.tellp();
		fileHeader.numFiles++;

		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<uint32_t>(static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles);
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));
		return 0;
	}

	long BinaryLoader::CreateFromFile(const char * filePath,const std::string& guid, const std::string & type)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		size_t index;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			index = findType->second;
			auto& files = typeIndexToFiles[index];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				return 1;
		}
		else
		{
			index = typeIndexToFiles.size();
			typeIndexToFiles.push_back({});
			typeToIndex[type] = static_cast<uint32_t>(index);
		}

		std::fstream fileIn (filePath, std::ios::binary | std::ios::ate | std::ios::in);
		if (!fileIn.is_open())
			return -2;

		uint64_t size = static_cast<uint64_t>(fileIn.tellg());
		auto& files = typeIndexToFiles[index];
		files[guid] = static_cast<uint32_t>(entries.guid.size());
		entries.guid.push_back(guid);
		entries.type.push_back(type);
		entries.rawSize.push_back(size);
		entries.size.push_back(size);
		entries.location.push_back(fileHeader.endOfFiles);
		entries.guid_str.push_back(guid);
		entries.type_str.push_back(type);

		fileIn.seekg(0);
		AddFile(size, fileIn);
		return 0;
	}
	long BinaryLoader::GetFilesOfType(Utilz::GUID type, std::vector<File>& files) const noexcept
	{
		StartProfile;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& filesMap = typeIndexToFiles[findType->second];
			files.reserve(filesMap.size());
			for (auto& f : filesMap)
			{
				files.push_back(
					{
						entries.guid[f.second],
						type,
						entries.guid_str[f.second],
						entries.type_str[f.second]
					});
			}
			return 0;
		}
		return 0;
	}
	long BinaryLoader::GetFiles(std::vector<File>& files) const noexcept
	{
		StartProfile;
		files.reserve(fileHeader.numFiles);
		for (uint32_t i = 0; i < fileHeader.numFiles; i++)
		{
			files.push_back({
				entries.guid[i],
				entries.type[i],
				entries.guid_str[i],
				entries.type_str[i]
				});
		}
		return 0;
	}
	long BinaryLoader::GetFiles(FILE_C * files, uint32_t numfiles) const noexcept
	{ 
		StartProfile;
		if (numfiles != fileHeader.numFiles)
			return -1;

		auto& f = files;
		for (uint32_t i = 0; i < fileHeader.numFiles; i++)
		{
			f[i].guid = entries.guid[i].id;
			f[i].type = entries.type[i].id;
			f[i].guid_str = entries.guid_str[i].c_str();
			f[i].type_str = entries.type_str[i].c_str();
		}
		return 0;
	}
	float BinaryLoader::GetFragmentationRatio() const noexcept
	{
		return (float)fileHeader.unusedSpace / (float)(fileHeader.endOfFiles - sizeof(fileHeader));
	}
	void BinaryLoader::AddFile(uint64_t size, void* data)
	{
		StartProfile;
		file.seekp(fileHeader.endOfFiles);
		file.write((char*)data, size);

		fileHeader.endOfFiles = file.tellp();
		fileHeader.numFiles++;

		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<uint32_t>( static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles);
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));
	}

	void BinaryLoader::AddFile(uint64_t size, std::fstream & in)
	{
		StartProfile;
		file.seekp(fileHeader.endOfFiles);
		CopyFile(in, file, 0, fileHeader.endOfFiles, size);

		fileHeader.endOfFiles = file.tellp();
		fileHeader.numFiles++;

		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<uint32_t>(static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles);
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));
	}

	void BinaryLoader::RemoveFile(uint32_t index)
	{
		StartProfile;
		size_t last = fileHeader.numFiles - 1;
		fileHeader.unusedSpace += entries.rawSize[index];
		if (last != index)
		{
			/*if (index == entries.newLastIndex[last])
			{
				CopyFile(file, file, entries.location[last], entries.location[index], entries.rawSize[last]);
				fileHeader.endOfFiles -= entries.rawSize[index];
				entries.location[last] = entries.location[index];
				fileHeader.lastIndex = index;
			}
			else if(entries.rawSize[last] <= entries.rawSize[index])
			{
				CopyFile(file, file, entries.location[last], entries.location[index], entries.rawSize[last]);
				fileHeader.endOfFiles -= entries.rawSize[last];
				entries.location[last] = entries.location[index];
				fileHeader.lastIndex = entries.newLastIndex[last];
				if(entries.rawSize[index] > entries.rawSize[last])
					fileHeader.unusedSpace += entries.rawSize[index] - entries.rawSize[last];
			}
			else
			{
				fileHeader.lastIndex = index;
				entries.newLastIndex[index] = entries.newLastIndex[last];
				fileHeader.unusedSpace += entries.rawSize[index];
			}*/
			entries.guid[index] = entries.guid[last];
			entries.type[index] = entries.type[last];
			entries.rawSize[index] = entries.rawSize[last];
			entries.size[index] = entries.size[last];
			entries.location[index] = entries.location[last];	
			entries.guid_str[index] = entries.guid_str[last];
			entries.type_str[index] = entries.type_str[last];

			auto& files = typeIndexToFiles[typeToIndex[entries.type[index]]];
			files[entries.guid[index]] = index;
		}

		entries.guid.pop_back();
		entries.type.pop_back();
		entries.rawSize.pop_back();
		entries.size.pop_back();
		entries.location.pop_back();
		entries.guid_str.pop_back();
		entries.type_str.pop_back();
	
		if (fileHeader.numFiles-- == 1)
			fileHeader.endOfFiles = sizeof(fileHeader);

		file.seekp(fileHeader.endOfFiles);
		
		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<uint32_t>(file.tellp()) - static_cast<uint32_t>(fileHeader.endOfFiles);
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));

#ifdef _WIN32
		HANDLE file = CreateFile(filePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		SetFilePointer(file, LONG(fileHeader.endOfFiles + fileHeader.tailSize), 0, FILE_BEGIN);
		SetEndOfFile(file);
#endif
	}


	void BinaryLoader::ReadTail()
	{
		StartProfile;
		if (fileHeader.numFiles)
		{
			entries.guid.resize(fileHeader.numFiles);
			entries.type.resize(fileHeader.numFiles);
			entries.rawSize.resize(fileHeader.numFiles);
			entries.size.resize(fileHeader.numFiles);
			entries.location.resize(fileHeader.numFiles);
			entries.guid_str.resize(fileHeader.numFiles);
			entries.type_str.resize(fileHeader.numFiles);

			file.read((char*)entries.guid.data(),		sizeof(entries.guid[0])		* fileHeader.numFiles);
			file.read((char*)entries.type.data(),		sizeof(entries.type[0])		* fileHeader.numFiles);
			file.read((char*)entries.rawSize.data(),	sizeof(entries.rawSize[0])	* fileHeader.numFiles);
			file.read((char*)entries.size.data(),		sizeof(entries.size[0])		* fileHeader.numFiles);
			file.read((char*)entries.location.data(),	sizeof(entries.location[0]) * fileHeader.numFiles);

			for (auto& f : entries.guid_str)
				readString(file, f);
			for (auto& t : entries.type_str)
				readString(file, t);
			for (uint32_t i = 0; i < fileHeader.numFiles; i++)
			{
				if (auto findType = typeToIndex.find(entries.type[i]); findType == typeToIndex.end())
					typeIndexToFiles.push_back({});
				auto& files = typeIndexToFiles[typeToIndex[entries.type[i]]];
				files[entries.guid[i]] = i;
			}
		}
	}

	long BinaryLoader::Init(const char* filePath, Mode mode)noexcept
	{
		StartProfile;
		this->filePath = filePath;
		this->mode = mode;
		auto m = std::ios::in | std::ios::binary | std::ios::ate;
		if (mode == Mode::EDIT)
			m |= std::ios::out;
		file.open(filePath, m);
		if (!file.is_open())
		{
			if (fs::exists(filePath))
				return -1;
			std::ofstream out(filePath, std::ios::binary);
			if (!out.is_open())
				return -2;
			fileHeader.endOfFiles = sizeof(fileHeader);
			fileHeader.tailSize = 0;
			fileHeader.numFiles = 0;
			fileHeader.version = version;
			fileHeader.unusedSpace = 0;
			out.write((char*)&fileHeader, sizeof(fileHeader));
			out.close();
			file.open(filePath, m);
			if (!file.is_open())
				return -3;
			return 0;
		}

		size_t totalFileSize = file.tellg();
		if (totalFileSize < sizeof(fileHeader))
		{
			file.close();
			std::ofstream out(filePath, std::ios::binary);
			if (!out.is_open())
				return -2;
			fileHeader.endOfFiles = sizeof(fileHeader);
			fileHeader.tailSize = 0;
			fileHeader.numFiles = 0;
			out.write((char*)&fileHeader, sizeof(fileHeader));
			out.close();
			file.open(filePath, m);
			if (!file.is_open())
				return -3;
			return 0;
		}

		file.seekg(0);
		file.read((char*)&fileHeader, sizeof(fileHeader));
		
		if (fileHeader.endOfFiles > totalFileSize)
			return -4;
		if (fileHeader.endOfFiles + fileHeader.tailSize > totalFileSize)
			return -5;

		file.seekg(fileHeader.endOfFiles);
		ReadTail();
		
		return 0;
	}
	long BinaryLoader::Shutdown()noexcept
	{
		StartProfile;
		file.close();
		typeIndexToFiles.clear();
		typeToIndex.clear();

		return 0;
	}
	long BinaryLoader::FindType(Utilz::GUID guid, Utilz::GUID& type)const noexcept
	{
		StartProfile;
		for (auto& files : typeIndexToFiles)
		{
			if (auto find = files.find(guid); find != files.end())
			{
				type = entries.type[find->second];
				return 0;
			}
		}
		return -1;
	}
	long BinaryLoader::FindNameAndType(Utilz::GUID guid, Utilz::GUID& name, Utilz::GUID& type)const noexcept
	{
		StartProfile;
		for (auto& files : typeIndexToFiles)
		{
			if (auto find = files.find(guid); find != files.end())
			{
				name = entries.guid[find->second];
				type = entries.type[find->second];
				return 0;
			}
		}
		return -1;
	}
	
	long BinaryLoader::Exist(Utilz::GUID guid, Utilz::GUID type) const noexcept
	{
		StartProfile;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				return 1;
		}			
		return 0;
	}
	long BinaryLoader::GetSizeOfFile(Utilz::GUID guid, Utilz::GUID type, uint64_t& size) const noexcept
	{
		StartProfile;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
			{
				size = entries.size[findEntry->second];
				return 0;
			}
		}
		return -1;
	}
	long BinaryLoader::Read(Utilz::GUID guid, Utilz::GUID type,const ResourceDataVoid & data) noexcept
	{
		StartProfile;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
			{
				if (data.size > entries.size[findEntry->second])
					return -1;

				file.seekg(entries.location[findEntry->second]);
				file.read((char*)data.data, data.size);
				return 0;
			}
		}
		return -2;
	}
	long BinaryLoader::Create(const std::string& guid, const std::string& type, const ResourceDataVoid & data)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		size_t index;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			index = findType->second;
			auto& files = typeIndexToFiles[index];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				return 1;				
		}
		else
		{
			index = typeIndexToFiles.size();
			typeIndexToFiles.push_back({});
			typeToIndex[type] = static_cast<uint32_t>(index);
		}
		auto& files = typeIndexToFiles[index];
		files[guid] = static_cast<uint32_t>(entries.guid.size());
		entries.guid.push_back(guid);
		entries.type.push_back(type);
		entries.rawSize.push_back(data.size);
		entries.size.push_back(data.size);
		entries.location.push_back(fileHeader.endOfFiles);
		entries.guid_str.push_back(guid);
		entries.type_str.push_back(type);
		AddFile(data.size, data.data);
		return 0;
	}
	long BinaryLoader::Destroy(Utilz::GUID guid, Utilz::GUID type)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;

		if (const auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (const auto findEntry = files.find(guid); findEntry != files.end())
			{		
				RemoveFile(findEntry->second);
				files.erase(guid);
				return 0;
			}
		}
		return 1;
	}
	long BinaryLoader::Defrag()noexcept
	{
		std::ofstream out("data.temp", std::ios::binary | std::ios::trunc);
		if (!out.is_open())
			return -1;

		out.seekp(sizeof(fileHeader));
		for (size_t i = 0; i < fileHeader.numFiles; i++)
		{
			size_t newLocation = out.tellp();
			CopyFile(file, out, entries.location[i], newLocation, entries.rawSize[i]);
			entries.location[i] = newLocation;
		}

		fileHeader.endOfFiles = out.tellp();
		WriteTail(out, entries, fileHeader.numFiles);
		out.seekp(0);
		fileHeader.unusedSpace = 0;
		fileHeader.version = version;
		out.write((char*)&fileHeader, sizeof(fileHeader));

		out.close();
		file.close();
		fs::remove(filePath);
		fs::rename("data.temp", filePath);
		fs::remove("data.temp");
		auto m = std::ios::in | std::ios::binary | std::ios::ate;
		if (mode == Mode::EDIT)
			m |= std::ios::out;
		file.open(filePath, m);
		
		return 0;
	}
	uint32_t BinaryLoader::GetNumberOfFiles()const noexcept
	{
		return fileHeader.numFiles;
	}
	uint32_t BinaryLoader::GetNumberOfTypes()const noexcept
	{
		return static_cast<uint32_t>(typeToIndex.size());
	}
	uint64_t BinaryLoader::GetTotalSizeOfAllFiles()const noexcept
	{
		return fileHeader.endOfFiles - sizeof(fileHeader);
	}
}