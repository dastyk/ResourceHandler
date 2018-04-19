#include "BinaryFileSystem.h"
#include <Profiler.h>
#include <optional>
#include <filesystem>
#include <fstream>
#include <StringReadWrite.h>
namespace fs = std::experimental::filesystem;
#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
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
				Utilities::writeString(&file, s);
			for (auto& t : entries.type_str)
				Utilities::writeString(&file, t);
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


	
	UERROR BinaryFileSystem::GetFilesOfType(Utilities::GUID type, FILE_C files[], uint32_t numFiles) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto find = typeToIndex.find(type); find == typeToIndex.end())
			RETURN_ERROR("Could not find type: ");
		else
		{
			if (numFiles > uint32_t(typeIndexToFiles[find->second].size()))
				RETURN_ERROR("Array size out of bounds");
			uint32_t i = 0;
			for (auto& f : typeIndexToFiles[find->second])
			{
				files[i].guid = entries.guid[f.second].id;
				files[i].type = entries.type[f.second].id;
				files[i].guid_str = entries.guid_str[f.second].c_str();
				files[i].type_str = entries.type_str[f.second].c_str();
				++i;
			}
			RETURN_SUCCESS;
		}
	}

	UERROR BinaryFileSystem::CreateFromCallback(const std::string & guid, const std::string & type, const std::function<bool(std::ostream* file)>& function)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");

		Locker lg(lock, mode);
		size_t index;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			index = findType->second;
			auto& files = typeIndexToFiles[index];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				RETURN_ERROR("File already exists");
		}
		else
		{
			index = typeIndexToFiles.size();
			typeIndexToFiles.push_back({});
			typeToIndex[type] = static_cast<uint32_t>(index);
		}


		file.seekp(fileHeader.endOfFiles);
		if (!function(&file))
			RETURN_ERROR("Write callback failed");
	
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
		RETURN_SUCCESS;
	}

	UERROR BinaryFileSystem::CreateFromFile(const char * filePath,const std::string& guid, const std::string & type)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");
		Locker lg(lock, mode);
		size_t index;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			index = findType->second;
			auto& files = typeIndexToFiles[index];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				RETURN_ERROR("File already exists");
		}
		else
		{
			index = typeIndexToFiles.size();
			typeIndexToFiles.push_back({});
			typeToIndex[type] = static_cast<uint32_t>(index);
		}

		std::fstream fileIn (filePath, std::ios::binary | std::ios::ate | std::ios::in);
		if (!fileIn.is_open())
			RETURN_ERROR("Could not open input file");

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
		RETURN_SUCCESS;
	}
	UERROR BinaryFileSystem::GetFilesOfType(Utilities::GUID type, std::vector<File>& files) const noexcept
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
			RETURN_SUCCESS;
		}
		RETURN_ERROR("No files found");
	}
	UERROR BinaryFileSystem::GetFiles(std::vector<File>& files) const noexcept
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
		RETURN_SUCCESS;
	}
	UERROR BinaryFileSystem::GetFiles(FILE_C * files, uint32_t numfiles) const noexcept
	{ 
		StartProfile;
		Locker lg(lock, mode);
		if (numfiles > fileHeader.numFiles)
			RETURN_ERROR("Array out of bound");

		auto& f = files;
		for (uint32_t i = 0; i < fileHeader.numFiles; i++)
		{
			f[i].guid = entries.guid[i].id;
			f[i].type = entries.type[i].id;
			f[i].guid_str = entries.guid_str[i].c_str();
			f[i].type_str = entries.type_str[i].c_str();
		}
		RETURN_SUCCESS;
	}
	float BinaryFileSystem::GetFragmentationRatio() const noexcept
	{
		Locker lg(lock, mode);
		if (fileHeader.endOfFiles - sizeof(fileHeader) > 0)
			return (float)fileHeader.unusedSpace / (float)(fileHeader.endOfFiles - sizeof(fileHeader));
		else
			return 0.0f;
	}
	uint32_t BinaryFileSystem::GetNumberOfFilesOfType(Utilities::GUID type) const noexcept
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
	UERROR BinaryFileSystem::GetFile(FILE_C & files, Utilities::GUID guid, Utilities::GUID type) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType == typeToIndex.end())
			RETURN_ERROR("Type not found");
		else
		{
			if (auto findFile = typeIndexToFiles[findType->second].find(guid); findFile == typeIndexToFiles[findType->second].end())
				RETURN_ERROR("File not found");
			else
			{
				files.guid = entries.guid[findFile->second].id;
				files.type = entries.type[findFile->second].id;
				files.guid_str = entries.guid_str[findFile->second].c_str();
				files.type_str = entries.type_str[findFile->second].c_str();
				RETURN_SUCCESS;
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
		HANDLE file = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
				Utilities::readString(&file, f);
			for (auto& t : entries.type_str)
				Utilities::readString(&file, t);
			for (uint32_t i = 0; i < fileHeader.numFiles; i++)
			{
				
				if (auto findType = typeToIndex.find(entries.type[i]); findType == typeToIndex.end())
				{
					uint32_t index = uint32_t(typeIndexToFiles.size());
					typeIndexToFiles.push_back({});
					typeToIndex[entries.type[i]] = index;
				}
					
				auto& files = typeIndexToFiles[typeToIndex[entries.type[i]]];
				files[entries.guid[i]] = i;
			}
		}
	}

	UERROR BinaryFileSystem::Init(const char* filePath, Mode mode)noexcept
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
				RETURN_ERROR("SystemFile could not be accessed");
			std::ofstream out(filePath, std::ios::binary);
			if (!out.is_open())
				RETURN_ERROR("Could not create SystemFile");
			fileHeader.endOfFiles = sizeof(fileHeader);
			fileHeader.tailSize = 0;
			fileHeader.numFiles = 0;
			fileHeader.version = version;
			fileHeader.unusedSpace = 0;
			out.write((char*)&fileHeader, sizeof(fileHeader));
			out.close();
			file.open(filePath, m);
			if (!file.is_open())
				RETURN_ERROR("Could not read new SystemFile");
			RETURN_SUCCESS;
		}

		size_t totalFileSize = size_t(file.tellg());
		if (totalFileSize < sizeof(fileHeader))
			RETURN_ERROR("The SystemFile is corrupted");

		file.seekg(0);
		file.read((char*)&fileHeader, sizeof(fileHeader));
		
		if (fileHeader.endOfFiles > totalFileSize)
			RETURN_ERROR("The SystemFile is corrupted");
		if (fileHeader.endOfFiles + fileHeader.tailSize > totalFileSize)
			RETURN_ERROR("The System File is corrupted");

		file.seekg(fileHeader.endOfFiles);
		ReadTail();
		
		RETURN_SUCCESS;
	}
	UERROR BinaryFileSystem::Shutdown()noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		file.close();
		typeIndexToFiles.clear();
		typeToIndex.clear();

		RETURN_SUCCESS;
	}
	UERROR BinaryFileSystem::FindType(Utilities::GUID guid, Utilities::GUID& type)const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		for (auto& files : typeIndexToFiles)
		{
			if (auto find = files.find(guid); find != files.end())
			{
				type = entries.type[find->second];
				RETURN_SUCCESS;
			}
		}
		RETURN_ERROR("Type not found");
	}
	UERROR BinaryFileSystem::FindNameAndType(Utilities::GUID guid, Utilities::GUID& name, Utilities::GUID& type)const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		for (auto& files : typeIndexToFiles)
		{
			if (auto find = files.find(guid); find != files.end())
			{
				name = entries.guid[find->second];
				type = entries.type[find->second];
				RETURN_SUCCESS;
			}
		}
		RETURN_ERROR("File not found");
	}
	
	bool BinaryFileSystem::Exist(Utilities::GUID guid, Utilities::GUID type) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				return true;
		}			
		return false;
	}
	UERROR BinaryFileSystem::GetSizeOfFile(Utilities::GUID guid, Utilities::GUID type, uint64_t& size) const noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
			{
				size = entries.size[findEntry->second];
				RETURN_SUCCESS;
			}
		}
		RETURN_ERROR("File not found");
	}
	UERROR BinaryFileSystem::Read(Utilities::GUID guid, Utilities::GUID type,const ResourceDataVoid & data) noexcept
	{
		StartProfile;
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (auto findEntry = files.find(guid); findEntry != files.end())
			{
				if (data.size > entries.size[findEntry->second])
					RETURN_ERROR("Data size out of bound");
				lg.Lock();
				file.seekg(entries.location[findEntry->second]);
				file.read((char*)data.data, data.size);
				RETURN_SUCCESS;
			}
		}
		RETURN_ERROR("File not found");
	}
	UERROR BinaryFileSystem::Create(const std::string& guid, const std::string& type, const ResourceDataVoid & data)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");
		Locker lg(lock, mode);
		size_t index;
		if (auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			index = findType->second;
			auto& files = typeIndexToFiles[index];
			if (auto findEntry = files.find(guid); findEntry != files.end())
				RETURN_ERROR("File already exists");
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
		RETURN_SUCCESS;
	}

	UERROR BinaryFileSystem::Write(Utilities::GUID guid, Utilities::GUID type, const ResourceDataVoid & data)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType == typeToIndex.end())
			RETURN_ERROR("Type not found");
		else
		{
			if (auto findFile = typeIndexToFiles[findType->second].find(guid); findFile == typeIndexToFiles[findType->second].end())
				RETURN_ERROR("File not found");
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
					Create(namestr, typestr, data);
				}
			}
		}
		RETURN_SUCCESS;
	}

	UERROR BinaryFileSystem::WriteFromCallback(Utilities::GUID guid, Utilities::GUID type, uint64_t size, const std::function<bool(std::ostream*file)>& function)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");
		Locker lg(lock, mode);
		if (auto findType = typeToIndex.find(type); findType == typeToIndex.end())
			RETURN_ERROR("Type not found");
		else
		{
			if (auto findFile = typeIndexToFiles[findType->second].find(guid); findFile == typeIndexToFiles[findType->second].end())
				RETURN_ERROR("File not found");
			else
			{
				if (size == entries.size[findFile->second])
				{
					file.seekp(entries.location[findFile->second]);
					if (!function(&file))
						RETURN_ERROR("UERROR in write callback");
					uint64_t asize = static_cast<uint64_t>(file.tellp()) - entries.location[findFile->second];
					if (size != asize)
						RETURN_ERROR("CRITICAL: The write callback wrote to much and has likely corrupted another file");
					
				}
				else if (size < entries.size[findFile->second])
				{
					file.seekp(entries.location[findFile->second]);
					if (!function(&file))
						RETURN_ERROR("UERROR in write callback");
					uint64_t asize = static_cast<uint64_t>(file.tellp()) - entries.location[findFile->second];
					if (size != asize)
						RETURN_ERROR("CRITICAL: The write callback wrote to much and has likely corrupted another file");



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
		RETURN_SUCCESS;
	}
	UERROR BinaryFileSystem::Destroy(Utilities::GUID guid, Utilities::GUID type)noexcept
	{
		StartProfile;
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");

		Locker lg(lock, mode);
		if (const auto findType = typeToIndex.find(type); findType != typeToIndex.end())
		{
			auto& files = typeIndexToFiles[findType->second];
			if (const auto findEntry = files.find(guid); findEntry != files.end())
			{		
				RemoveFile(findEntry->second);
				files.erase(guid);
				RETURN_SUCCESS;
			}
		}
		RETURN_ERROR("File not found");
	}

	UERROR BinaryFileSystem::Defrag()noexcept
	{
		if (mode != Mode::EDIT)
			RETURN_ERROR("FileSystem not in edit mode");
		StartProfile;
		Locker lg(lock, mode);


		std::ofstream out("data.temp", std::ios::binary | std::ios::trunc);
		if (!out.is_open())
			RETURN_ERROR("Could not open temporary defrag file");

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
		if (file.is_open())
			RETURN_ERROR("Could not close system file");
		if (out.is_open())
			RETURN_ERROR("Could not close temporary defrag file");
	

		std::error_code err;
		fs::remove("data.temp2", err);
		fs::rename(filePath, "data.temp2", err);
		if (err)
			RETURN_ERROR("Could not rename system file");
		fs::rename("data.temp", filePath, err);
		if (err)
			RETURN_ERROR("Could not rename temp file");
		fs::remove("data.temp2", err);

		auto m = std::ios::in | std::ios::binary | std::ios::ate | std::ios::out;
		file.open(filePath, m);
		if (!file.is_open())
			RETURN_ERROR("Could not open new system file");
		RETURN_SUCCESS;
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