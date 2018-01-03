#include "pch.h"
#include <GUID.h>
#include "../Include/Utilz/ThreadPool.h"
#include <filesystem>
#include "../Include/ResourceHandler/FileSystem_Interface.h"
#include <fstream>
namespace fs = std::experimental::filesystem;
TEST(BinaryFileSystem, CreateDestroy) {
  
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);

		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);

		char file1Data[] = { "test" };
		auto size = sizeof(file1Data);
		r = CreateS_C(bl, "File1", "Test", file1Data, size);
		EXPECT_EQ(r, 0);
		r = CreateS_C(bl, "File1", "Test", file1Data, size);
		EXPECT_EQ(r, 1);
		
		char newfile1Data[sizeof(file1Data)];
		uint64_t newsize = 0;
		r = GetSizeOfFileS_C(bl, "File1", "Test", &newsize);
		EXPECT_EQ(newsize, size);
		r = ReadS_C(bl, "File1", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1Data);
		
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		r = DestroyS_C(bl, "File1", "Test");
		EXPECT_EQ(r, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 0);

		DestroyLoader(bl);
	}
	
	fs::remove("cd.dat", err);
}
TEST(BinaryFileSystem, CreateDestroyTwo) {

	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);

		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);

		char file1Data[] = { "test" };
		auto size = sizeof(file1Data);
		r = CreateS_C(bl, "File1", "Test", file1Data, size);
		EXPECT_EQ(r, 0);
		r = CreateS_C(bl, "File1", "Test", file1Data, size);
		EXPECT_EQ(r, 1);

		char newfile1Data[sizeof(file1Data)];
		uint64_t newsize = 0;
		r = GetSizeOfFileS_C(bl, "File1", "Test", &newsize);
		EXPECT_EQ(newsize, size);
		r = ReadS_C(bl, "File1", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1Data);

		char file2Data[] = { "test2" };
		auto size2 = sizeof(file2Data);
		r = CreateS_C(bl, "File2", "Test", file2Data, size2);
		EXPECT_EQ(r, 0);
		r = CreateS_C(bl, "File2", "Test", file2Data, size2);
		EXPECT_EQ(r, 1);
		EXPECT_EQ(GetNumberOfFilesOfType_C(bl, "Test"), 2);

		char newfile2Data[sizeof(file2Data)];
		uint64_t newsize2 = 0;
		r = GetSizeOfFileS_C(bl, "File2", "Test", &newsize2);
		EXPECT_EQ(newsize2, size2);
		r = ReadS_C(bl, "File2", "Test", newfile2Data, newsize2);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile2Data, file2Data);

		EXPECT_EQ(GetNumberOfFiles_C(bl), 2);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);
		r = DestroyS_C(bl, "File1", "Test");
		EXPECT_EQ(r, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);

		char newfile3Data[sizeof(file2Data)];
		uint64_t newsize3 = 0;
		r = GetSizeOfFileS_C(bl, "File2", "Test", &newsize3);
		EXPECT_EQ(newsize3, size2);
		r = ReadS_C(bl, "File2", "Test", newfile3Data, newsize3);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile3Data, file2Data);

		DestroyLoader(bl);
	}

	fs::remove("cd.dat", err);
}
TEST(BinaryFileSystem, CreateFromFile)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);

		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);

		std::ofstream out("test.txt", std::ios::binary);
		EXPECT_TRUE(out.is_open());
		char file1Data[] = { "test" };
		auto size = sizeof(file1Data);
		out.write(file1Data, size);
		out.close();
		EXPECT_FALSE(out.is_open());

		auto re = CreateFromFile_C(bl, "test.txt", "File", "Test");
		EXPECT_EQ(re, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);
		EXPECT_EQ(GetNumberOfFilesOfType_C(bl, "Test"), 1);
		EXPECT_EQ(GetNumberOfFilesOfType_C(bl, "Korv"), 0);
		char newfile1Data[sizeof(file1Data)];
		uint64_t newsize = 0;
		r = GetSizeOfFileS_C(bl, "File", "Test", &newsize);
		EXPECT_EQ(newsize, size);
		r = ReadS_C(bl, "File", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1Data);


		DestroyLoader(bl);
	}

	fs::remove("cd.dat", err);
}
TEST(BinaryFileSystem, CreateFromCallback)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);

		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);
		char file1Data[] = { "test" };
		auto size = sizeof(file1Data);
		auto lam = [&](std::ostream* file)
		{
			
			file->write(file1Data, size);
			return true;
		};

		auto re = bl->CreateFromCallback("File", "Test", lam);
		EXPECT_EQ(re, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);

		char newfile1Data[sizeof(file1Data)];
		uint64_t newsize = 0;
		r = GetSizeOfFileS_C(bl, "File", "Test", &newsize);
		EXPECT_EQ(newsize, size);
		r = ReadS_C(bl, "File", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1Data);


		DestroyLoader(bl);
	}

	fs::remove("cd.dat", err);
}
TEST(BinaryFileSystem, Defrag)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);
		int count = 100;
		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);
		Utilz::GUID f444 = "105";
		Utilz::GUID f606 = "131";
		std::string test = "Int";
		Utilz::GUID testG = test;
		Utilz::GUID testC("Int");

		for (int i = 0; i < count; i++)
		{
			auto str = "File" + std::to_string(i) + "asd";
			r = bl->Create(str, "Int", { &i, sizeof(i) });
			EXPECT_EQ(r, 0);
			int c = 0;
			r = bl->Read(str, "Int", { &c, sizeof(i) });
			EXPECT_EQ(r, 0);
			EXPECT_EQ(c, i);
		}
		struct TestS
		{
			int i1;
			int i2;
		};
		for (int i = 0; i < count; i++)
		{
			auto str = "FileTest" + std::to_string(i);
			TestS d{ i, i * 10 };
			r = bl->Create(str, "Int2", { &d, sizeof(d) });
			EXPECT_EQ(r, 0);
			TestS d2;
			r = bl->Read(str, "Int2", { &d2, sizeof(d2) });
			EXPECT_EQ(r, 0);
			EXPECT_EQ(d2.i1, d.i1);
			EXPECT_EQ(d2.i2, d.i2);
		}
		EXPECT_EQ(GetNumberOfFiles_C(bl), count*2);
		EXPECT_EQ(GetTotalSizeOfAllFiles_C(bl), float(sizeof(int) * count + sizeof(TestS) * count));
		for (int i = 0; i < count; i++)
		{
			auto str = "File" + std::to_string(i) + "asd";
			r = bl->Destroy(str, "Int");
			EXPECT_EQ(r, 0);
		}
		EXPECT_EQ(GetNumberOfFiles_C(bl), count);
		EXPECT_EQ(GetTotalSizeOfAllFiles_C(bl), float(sizeof(int) * count + sizeof(TestS) * count));
		EXPECT_FLOAT_EQ(GetFragmentationRatio_C(bl), (sizeof(int)*count) / float(sizeof(int) * count + sizeof(TestS) * count));
		r = Defrag_C(bl);
		EXPECT_EQ(r, 0);
		EXPECT_EQ(GetTotalSizeOfAllFiles_C(bl), sizeof(TestS) * count);
		EXPECT_EQ(GetFragmentationRatio_C(bl), 0.0f);
		DestroyLoader(bl);
	}

	fs::remove("cd.dat", err);
}
TEST(BinaryFileSystem, CreateAndWrite) {

	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);

		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);

		char file1Data[] = { "test" };
		auto size = sizeof(file1Data);
		r = CreateS_C(bl, "File1", "Test", file1Data, size);
		EXPECT_EQ(r, 0);
		r = CreateS_C(bl, "File1", "Test", file1Data, size);
		EXPECT_EQ(r, 1);

		char newfile1Data[sizeof(file1Data)];
		uint64_t newsize = 0;
		r = GetSizeOfFileS_C(bl, "File1", "Test", &newsize);
		EXPECT_EQ(newsize, size);
		r = ReadS_C(bl, "File1", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1Data);

		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);


		char file1DataS[] = { "asd" };
		auto sizeS = sizeof(file1DataS);
		r = WriteS_C(bl, "File1", "Test", file1DataS, sizeS);
		EXPECT_EQ(r, 0);

		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);


		r = GetSizeOfFileS_C(bl, "File1", "Test", &newsize);
		EXPECT_EQ(newsize, sizeS);
		r = ReadS_C(bl, "File1", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1DataS);

		char file1DataL[] = { "File12" };
		auto sizeL = sizeof(file1DataL);
		r = WriteS_C(bl, "File1", "Test", file1DataL, sizeL);
		EXPECT_EQ(r, 0);

		
		char newfile1DataL[sizeof(file1DataL)];
		r = GetSizeOfFileS_C(bl, "File1", "Test", &newsize);
		EXPECT_EQ(newsize, sizeL);
		r = ReadS_C(bl, "File1", "Test", newfile1DataL, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1DataL, file1DataL);

		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);

		r = DestroyS_C(bl, "File1", "Test");
		EXPECT_EQ(r, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 0);

		DestroyLoader(bl);
	}

	fs::remove("cd.dat", err);
}
TEST(BinaryFileSystem, CreateAndWriteFromCallback)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
		EXPECT_TRUE(bl);

		auto r = InitLoader_C(bl, "cd.dat", ResourceHandler::Mode::EDIT);
		EXPECT_EQ(r, 0);
		char file1Data[] = { "test" };
		auto size = sizeof(file1Data);
		auto lam = [&](std::ostream* file)
		{

			file->write(file1Data, size);
			return true;
		};

		auto re = bl->CreateFromCallback("File", "Test", lam);
		EXPECT_EQ(re, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);

		char newfile1Data[sizeof(file1Data)];
		uint64_t newsize = 0;
		r = GetSizeOfFileS_C(bl, "File", "Test", &newsize);
		EXPECT_EQ(newsize, size);
		r = ReadS_C(bl, "File", "Test", newfile1Data, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data, file1Data);

		char file1Data2[] = { "tes" };
		auto size2 = sizeof(file1Data2);
		auto lam2 = [&](std::ostream* file)
		{

			file->write(file1Data2, size2);
			return true;
		};


		 re = bl->WriteFromCallback("File", "Test", size2, lam2);
		EXPECT_EQ(re, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);

		char newfile1Data2[sizeof(file1Data2)];
		 newsize = 0;
		r = GetSizeOfFileS_C(bl, "File", "Test", &newsize);
		EXPECT_EQ(newsize, size2);
		r = ReadS_C(bl, "File", "Test", newfile1Data2, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data2, file1Data2);

		char file1Data22[] = { "test123" };
		auto size22 = sizeof(file1Data22);
		auto lam22 = [&](std::ostream* file)
		{

			file->write(file1Data22, size22);
			return true;
		};


		 re = bl->WriteFromCallback("File", "Test", size22, lam22);
		EXPECT_EQ(re, 0);
		EXPECT_EQ(GetNumberOfFiles_C(bl), 1);
		EXPECT_EQ(GetNumberOfTypes_C(bl), 1);

		char newfile1Data22[sizeof(file1Data22)];
		 newsize = 0;
		r = GetSizeOfFileS_C(bl, "File", "Test", &newsize);
		EXPECT_EQ(newsize, size22);
		r = ReadS_C(bl, "File", "Test", newfile1Data22, newsize);
		EXPECT_EQ(r, 0);
		EXPECT_STREQ(newfile1Data22, file1Data22);

		DestroyLoader(bl);
	}

	fs::remove("cd.dat", err);
}
TEST(GUID, std)
{
	std::string str = "asasdasd";
	uint32_t hash = 0;
	for (int i = 0; i < 20000; i++)
		hash = uint32_t(std::hash<std::string>{}(str));
}
TEST(GUID, GUIDRun)
{
	std::string str = "asasdasd";
	Utilz::GUID hash = 0;
	for (int i = 0; i < 20000; i++)
		hash = str;
}
TEST(GUID, GUIDCom)
{
	std::string str = "asasdasd";
	Utilz::GUID hash = 0;
	for (int i = 0; i < 20000; i++)
		hash = Utilz::GUID("asasdasd");
}
static int Chungus(int a, int b)
{
	return a + b;
}

struct X
{
	X(int i_) : i(i_) {  };
	int i;
	int Foo(int x)const
	{
		return i + x;
	}

	bool Do(Utilz::ThreadPool* tp)
	{
		auto res = tp->Enqueue(this, &X::Foo, 10);
		return res.get() == i + 10;
	}
};
TEST(ThreadPool, Various)
{
	Utilz::ThreadPool tp(4);
	{
		std::mutex lock;
		int someInt = 0;
		std::vector<std::future<void>> results;
		results.reserve(1000);
		for (int i = 0; i < 1000; ++i)
		{
			results.emplace_back(tp.Enqueue([&someInt, &lock]()
			{
				lock.lock();
				++someInt;
				lock.unlock();
			}));
		}

		for (int i = 0; i < 1000; ++i)
		{
			results[i].get();
		}
		EXPECT_EQ( someInt , 1000);
	}
	//Testing adding a lambda
	{
		auto someLambda = []() {return 5; };
		auto future = tp.Enqueue(someLambda);
		EXPECT_EQ(future.get() , 5);
	}

	//Testing a regular function.
	{
		auto future = tp.Enqueue(Chungus, 10, 15);
		EXPECT_EQ(future.get() , Chungus(10, 15));
	}

	// Testing methods
	{
		X x(10);
		auto ret = tp.Enqueue(&x, &X::Foo, 20);
		EXPECT_EQ(ret.get() , 20 + x.i);
		EXPECT_TRUE(x.Do(&tp));
	}
}