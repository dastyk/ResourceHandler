#include "BinaryFileSystem.h"
#include <Profiler.h>
#include <optional>
#include <filesystem>
#include <fstream>
#include <Utilz\StringReadWrite.h>
namespace fs = std::experimental::filesystem;
#ifdef _WIN32
#include <Windows.h>
#undef min
#undef CopyFile
#endif

namespace ResourceHandler
{
	struct Locker
	{
		std::recursive_mutex & mtx;
		Mode mode;
		bool locked = false;
		Locker(std::recursive_mutex& mtx, Mode mode) : mtx(mtx), mode(mode)
		{
			if (mode == Mode::EDIT)
			{
				locked = true;
				mtx.lock();
			}

		}
		inline void Lock()
		{
			if (!locked)
			{
				locked = true;
				mtx.lock();

			}

		}
		~Locker()
		{
			if (locked)
				mtx.unlock();
		}
	};
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
				Utilz::writeString(&file, s);
			for (auto& t : entries.type_str)
				Utilz::writeString(&file, t);
		}
	}

	void CopyFile(std::istream* in, std::ostream* out, uint64_t readPos, uint64_t writePos, uint64_t size)
	{
		StartProfile;
		uint64_t copied = 0;
		char buff[524288];
		while (copied < size)
		{
			size_t toWrite = size_t(std::min(uint64_t(524288), size - copied));
			copied += toWrite;
			in->seekg(readPos);
			in->read(buff, toWrite);
			out->seekp(writePos);
			out->write(buff, toWrite);
			writePos += toWrite;
			readPos += toWrite;
		}

	}
	void CopyFile(char* data, std::ostream* out, uint64_t readPos, uint64_t writePos, uint64_t size)
	{
		StartProfile;
		out->seekp(writePos);
		out->write(&data[readPos], size);

	}
	BinaryFileSystem::BinaryFileSystem()noexcept
	{
	}


	BinaryFileSystem::~BinaryFileSystem()noexcept
	{
	}


	
	long BinaryFileSystem::GetFilesOfType(Utilz::GUID type, FILE_C files[], uint32_t numFiles) const noexcept
	{
		Locker lg(lock, mode);
		if (auto find = typeToIndex.find(type); find == typeToIndex.end())
			return -1;
		else
		{
			if (numFiles != uint32_t(typeIndexToFiles[find->second].size()))
				return -2;
			uint32_t i = 0;
			for (auto& f : typeIndexToFiles[find->second])
			{
				files[i].guid = entries.guid[f.second].id;
				files[i].type = entries.type[f.second].id;
				files[i].guid_str = entries.guid_str[f.second].c_str();
				files[i].type_str = entries.type_str[f.second].c_str();
				++i;
			}
			return 0;
		}
	}

	long BinaryFileSystem::CreateFromCallback(const std::string & guid, const std::string & type, const std::function<bool(std::ostream* file)>& function)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;

		Locker lg(lock, mode);
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

	long BinaryFileSystem::CreateFromFile(const char * filePath,const std::string& guid, const std::string & type)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		Locker lg(lock, mode);
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
	long BinaryFileSystem::GetFilesOfType(Utilz::GUID type, std::vector<File>& files) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
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
		return -1;
	}
	long BinaryFileSystem::GetFiles(std::vector<File>& files) const noexcept
	{
		StartProfile;

		Locker lg(lock, mode);
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
	long BinaryFileSystem::GetFiles(FILE_C * files, uint32_t numfiles) const noexcept
	{ 
		StartProfile;
		Locker lg(lock, mode);
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
	float BinaryFileSystem::GetFragmentationRatio() const noexcept
	{
		Locker lg(lock, mode);
		return (float)fileHeader.unusedSpace / (float)(fileHeader.endOfFiles - sizeof(fileHeader));
	}
	uint32_t BinaryFileSystem::GetNumberOfFilesOfType(Utilz::GUID type) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto find = typeToIndex.find(type); find == typeToIndex.end())
			return 0;
		else
		{
			return uint32_t(typeIndexToFiles[find->second].size());
		}
	}
	long BinaryFileSystem::GetFile(FILE_C & files, Utilz::GUID guid, Utilz::GUID type) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType == typeToIndex.end())
			return -1;
		else
		{
			if (auto findFile = typeIndexToFiles[findType->second].find(guid); findFile == typeIndexToFiles[findType->second].end())
				return -2;
			else
			{
				files.guid = entries.guid[findFile->second].id;
				files.type = entries.type[findFile->second].id;
				files.guid_str = entries.guid_str[findFile->second].c_str();
				files.type_str = entries.type_str[findFile->second].c_str();
				return 0;
			}
		}
	}
	void BinaryFileSystem::AddFile(uint64_t size, void* data)
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

	void BinaryFileSystem::AddFile(uint64_t size, std::fstream & in)
	{
		StartProfile;
		file.seekp(fileHeader.endOfFiles);
		CopyFile(&in, &file, 0, fileHeader.endOfFiles, size);

		fileHeader.endOfFiles = file.tellp();
		fileHeader.numFiles++;

		WriteTail(file, entries, fileHeader.numFiles);
		fileHeader.tailSize = static_cast<uint32_t>(static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles);
		file.seekp(0);
		file.write((char*)&fileHeader, sizeof(fileHeader));
	}

	void BinaryFileSystem::RemoveFile(uint32_t index)
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


	void BinaryFileSystem::ReadTail()
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
				Utilz::readString(&file, f);
			for (auto& t : entries.type_str)
				Utilz::readString(&file, t);
			for (uint32_t i = 0; i < fileHeader.numFiles; i++)
			{
				if (auto findType = typeToIndex.find(entries.type[i]); findType == typeToIndex.end())
					typeIndexToFiles.push_back({});
				auto& files = typeIndexToFiles[typeToIndex[entries.type[i]]];
				files[entries.guid[i]] = i;
			}
		}
	}

	long BinaryFileSystem::Init(const char* filePath, Mode mode)noexcept
	{
		StartProfile;
		this->filePath = filePath;
		this->mode = mode;
		auto m = std::ios::in | std::ios::binary | std::ios::ate;
		if (mode == Mode::EDIT)
			m |= std::ios::out;
		Locker lg(lock, mode);
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

		size_t totalFileSize = size_t(file.tellg());
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
	long BinaryFileSystem::Shutdown()noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		file.close();
		typeIndexToFiles.clear();
		typeToIndex.clear();

		return 0;
	}
	long BinaryFileSystem::FindType(Utilz::GUID guid, Utilz::GUID& type)const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
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
	long BinaryFileSystem::FindNameAndType(Utilz::GUID guid, Utilz::GUID& name, Utilz::GUID& type)const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
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
	
	long BinaryFileSystem::Exist(Utilz::GUID guid, Utilz::GUID type) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				return 1;
		}			
		return 0;
	}
	long BinaryFileSystem::GetSizeOfFile(Utilz::GUID guid, Utilz::GUID type, uint64_t& size) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
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
	long BinaryFileSystem::Read(Utilz::GUID guid, Utilz::GUID type,const ResourceDataVoid & data) noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
			{
				if (data.size > entries.size[findEntry->second])
					return -1;
				lg.Lock();
				file.seekg(entries.location[findEntry->second]);
				file.read((char*)data.data, data.size);
				return 0;
			}
		}
		return -2;
	}
	long BinaryFileSystem::Create(const std::string& guid, const std::string& type, const ResourceDataVoid & data)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		Locker lg(lock, mode);
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

	long BinaryFileSystem::Write(Utilz::GUID guid, Utilz::GUID type, const ResourceDataVoid & data)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType == typeToIndex.end())
			return -2;
		else
		{
			if (auto findFile = typeIndexToFiles[findType->second].find(guid); findFile == typeIndexToFiles[findType->second].end())
				return -3;
			else
			{
				if (data.size == entries.size[findFile->second])
				{
					CopyFile((char*)data.data, &file, 0, entries.location[findFile->second], data.size);

					
				}
				else if (data.size < entries.size[findFile->second])
				{
					CopyFile((char*)data.data, &file, 0, entries.location[findFile->second], data.size);

					fileHeader.unusedSpace += entries.size[findFile->second] - data.size;
					entries.size[findFile->second] = data.size;
					entries.rawSize[findFile->second] = data.size;

					WriteTail(file, entries, fileHeader.numFiles);
					fileHeader.tailSize = static_cast<uint32_t>(static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles);
					file.seekp(0);
					file.write((char*)&fileHeader, sizeof(fileHeader));
				}
				else
				{
					auto namestr = entries.guid_str[findFile->second];
					auto typestr = entries.type_str[findFile->second];
					Destroy(guid, type);
					Create(namestr, typestr, data);
				}
			}
		}
		return 0;
	}
	long BinaryFileSystem::WriteFromCallback(Utilz::GUID guid, Utilz::GUID type, uint64_t size, const std::function<bool(std::ostream*file)>& function)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType == typeToIndex.end())
			return -2;
		else
		{
			if (auto findFile = typeIndexToFiles[findType->second].find(guid); findFile == typeIndexToFiles[findType->second].end())
				return -3;
			else
			{
				if (size == entries.size[findFile->second])
				{
					file.seekp(entries.location[findFile->second]);
					if (!function(&file))
						return -4;
					uint64_t asize = static_cast<uint64_t>(file.tellp()) - entries.location[findFile->second];
					if (size != asize)
						return -5;
					

				}
				else if (size < entries.size[findFile->second])
				{
					file.seekp(entries.location[findFile->second]);
					if (!function(&file))
						return -4;
					uint64_t asize = static_cast<uint64_t>(file.tellp()) - entries.location[findFile->second];
					if (size != asize)
						return -5;


					fileHeader.unusedSpace += entries.size[findFile->second] - size;
					entries.size[findFile->second] = size;
					entries.rawSize[findFile->second] = size;

					file.seekp(fileHeader.endOfFiles);

					WriteTail(file, entries, fileHeader.numFiles);
					fileHeader.tailSize = static_cast<uint32_t>(static_cast<uint64_t>(file.tellp()) - fileHeader.endOfFiles);
					file.seekp(0);
					file.write((char*)&fileHeader, sizeof(fileHeader));
				}
				else
				{
					auto namestr = entries.guid_str[findFile->second];
					auto typestr = entries.type_str[findFile->second];
					Destroy(guid, type);
					CreateFromCallback(namestr, typestr, function);
				}
			}
		}
		return 0;
	}
	long BinaryFileSystem::Destroy(Utilz::GUID guid, Utilz::GUID type)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			return -1;

		Locker lg(lock, mode);
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
	long BinaryFileSystem::Defrag()noexcept
	{
		if (mode != Mode::EDIT)
			return -1;
		StartProfile;
		Locker lg(lock, mode);
		std::ofstream out("data.temp", std::ios::binary | std::ios::trunc);
		if (!out.is_open())
			return -2;

		out.seekp(sizeof(fileHeader));
		for (size_t i = 0; i < fileHeader.numFiles; i++)
		{
			uint64_t newLocation = out.tellp();
			CopyFile(&file, &out, entries.location[i], newLocation, entries.rawSize[i]);
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
		auto m = std::ios::in | std::ios::binary | std::ios::ate | std::ios::out;
		file.open(filePath, m);
		
		return 0;
	}
	uint32_t BinaryFileSystem::GetNumberOfFiles()const noexcept
	{
		Locker lg(lock, mode);
		return fileHeader.numFiles;
	}
	uint32_t BinaryFileSystem::GetNumberOfTypes()const noexcept
	{
		Locker lg(lock, mode);
		return static_cast<uint32_t>(typeToIndex.size());
	}
	uint64_t BinaryFileSystem::GetTotalSizeOfAllFiles()const noexcept
	{
		Locker lg(lock, mode);
		return fileHeader.endOfFiles - sizeof(fileHeader);
	}
}