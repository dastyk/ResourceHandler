#include "BinaryLoader.h"
#include <Profiler.h>
#include <optional>
#ifdef _WIN32
#include <Windows.h>
#undef min
#endif

namespace ResourceHandler
{
	template<class FILE, class TAIL>
	void WriteTail(FILE& file, TAIL& entries, size_t numFiles)
	{
		if (numFiles)
		{
			file.write((char*)entries.guid.data(),			sizeof(entries.guid[0])		* numFiles);
			file.write((char*)entries.type.data(),			sizeof(entries.type[0])		* numFiles);
			file.write((char*)entries.rawSize.data(),		sizeof(entries.rawSize[0])	* numFiles);
			file.write((char*)entries.size.data(),			sizeof(entries.size[0])		* numFiles);
			file.write((char*)entries.location.data(),		sizeof(entries.location[0]) * numFiles);
		}
	}

	BinaryLoader::BinaryLoader()noexcept
	{
	}


	BinaryLoader::~BinaryLoader()noexcept
	{
	}
	void BinaryLoader::AddFile(size_t size, void* data)
	{
		StartProfile;
		file.seekp(fileHeader.endOfFiles);
		file.write((char*)data, size);

		fileHeader.endOfFiles = file.tellp();
		fileHeader.numFiles++;

		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<size_t>(file.tellp()) - fileHeader.endOfFiles;
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));
	}

	void BinaryLoader::RemoveFile(size_t index)
	{
		size_t last = entries.guid.size() - 1;
		if (last != index)
		{
			if (entries.rawSize[last] <= entries.rawSize[index])
			{
				auto writePos = entries.location[index];
				auto readPos = entries.location[last];
				size_t copied = 0;
				char buff[1048576];
				while (copied < entries.rawSize[index])
				{
					size_t toWrite = std::min(size_t(1048576u), entries.rawSize[index] - copied);
					copied += toWrite;
					file.seekg(readPos);			
					file.read(buff, toWrite);
					file.seekp(writePos);
					file.write(buff, toWrite);
					writePos += toWrite;
					readPos += toWrite;
				}


				fileHeader.endOfFiles = writePos;
			}

			entries.guid[index] = entries.guid[last];
			entries.type[index] = entries.type[last];
			entries.rawSize[index] = entries.rawSize[last];
			entries.size[index] = entries.size[last];
			entries.location[index] = entries.location[last];	

			auto& files = typeIndexToFiles[typeToIndex[entries.type[index]]];
			files[entries.guid[index]] = index;
		}

		entries.guid.pop_back();
		entries.type.pop_back();
		entries.rawSize.pop_back();
		entries.size.pop_back();
		entries.location.pop_back();

		file.seekp(fileHeader.endOfFiles);
		fileHeader.numFiles--;

		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<size_t>(file.tellp()) - fileHeader.endOfFiles;
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));

#ifdef _WIN32
		HANDLE file = CreateFile("data.dat", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		SetFilePointer(file, LONG(fileHeader.endOfFiles + fileHeader.tailSize), 0, FILE_BEGIN);
		SetEndOfFile(file);
#endif
	}


	void BinaryLoader::ReadTail()
	{
		if (fileHeader.numFiles)
		{
			entries.guid.resize(fileHeader.numFiles);
			entries.type.resize(fileHeader.numFiles);
			entries.rawSize.resize(fileHeader.numFiles);
			entries.size.resize(fileHeader.numFiles);
			entries.location.resize(fileHeader.numFiles);

			file.read((char*)entries.guid.data(),		sizeof(entries.guid[0])		* fileHeader.numFiles);
			file.read((char*)entries.type.data(),		sizeof(entries.type[0])		* fileHeader.numFiles);
			file.read((char*)entries.rawSize.data(),	sizeof(entries.rawSize[0])	* fileHeader.numFiles);
			file.read((char*)entries.size.data(),		sizeof(entries.size[0])		* fileHeader.numFiles);
			file.read((char*)entries.location.data(),	sizeof(entries.location[0]) * fileHeader.numFiles);

			for (size_t i = 0; i < fileHeader.numFiles; i++)
			{
				if (auto findType = typeToIndex.find(entries.type[i]); findType == typeToIndex.end())
					typeIndexToFiles.push_back({});
				auto& files = typeIndexToFiles[typeToIndex[entries.type[i]]];
				files[entries.guid[i]] = i;
			}
		}
	}

	long BinaryLoader::Init(Mode mode)noexcept
	{
		StartProfile;
		this->mode = mode;
		auto m = std::ios::in | std::ios::binary | std::ios::ate;
		if (mode == Mode::EDIT)
			m |= std::ios::out;
		file.open("data.dat", m);
		if (!file.is_open())
		{
			std::ofstream out("data.dat", std::ios::binary);
			if (!out.is_open())
				return -1;
			fileHeader.endOfFiles = sizeof(fileHeader);
			fileHeader.tailSize = 0;
			fileHeader.numFiles = 0;
			out.write((char*)&fileHeader, sizeof(fileHeader));
			out.close();
			file.open("data.dat", m);
			if (!file.is_open())
				return -2;
			return 0;
		}

		size_t totalFileSize = file.tellg();
		if (totalFileSize < sizeof(fileHeader))
		{
			file.close();
			std::ofstream out("data.dat", std::ios::binary);
			if (!out.is_open())
				return -1;
			fileHeader.endOfFiles = sizeof(fileHeader);
			fileHeader.tailSize = 0;
			fileHeader.numFiles = 0;
			out.write((char*)&fileHeader, sizeof(fileHeader));
			out.close();
			file.open("data.dat", m);
			if (!file.is_open())
				return -2;
			return 0;
		}

		file.seekg(0);
		file.read((char*)&fileHeader, sizeof(fileHeader));
		
		if (fileHeader.endOfFiles > totalFileSize)
			return -3;
		if (fileHeader.endOfFiles + fileHeader.tailSize > totalFileSize)
			return -4;

		file.seekg(fileHeader.endOfFiles);
		ReadTail();
		
		return 0;
	}
	long BinaryLoader::Shutdown()noexcept
	{
		StartProfile;
		return 0;
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
	long BinaryLoader::Read(Utilz::GUID guid, Utilz::GUID type, ResourceData & data) noexcept
	{
		StartProfile;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
			{
				file.seekg(entries.location[findEntry->second]);
				data.size = entries.rawSize[findEntry->second];
				data.data = operator new(data.size);
				file.read((char*)data.data, data.size);
				return 0;
			}
		}
		return -1;
	}
	long BinaryLoader::Create(Utilz::GUID guid, Utilz::GUID type, const ResourceData & data)noexcept
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
			typeToIndex[type] = index;
		}
		auto& files = typeIndexToFiles[index];
		files[guid] = entries.guid.size();
		entries.guid.push_back(guid);
		entries.type.push_back(type);
		entries.rawSize.push_back(data.size);
		entries.size.push_back(data.size);
		entries.location.push_back(fileHeader.endOfFiles);
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

		return 0;
	}
	size_t BinaryLoader::GetNumberOfFiles()const noexcept
	{
		return fileHeader.numFiles;
	}
	size_t BinaryLoader::GetNumberOfTypes()const noexcept
	{
		return typeToIndex.size();
	}
}