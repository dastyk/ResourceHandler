#include "pch.h"
#include <GUID.h>
#include "../Include/Utilz/ThreadPool.h"
#include <filesystem>
#include "../Include/ResourceHandler/Loader_Interface.h"
#include <fstream>
namespace fs = std::experimental::filesystem;
TEST(BinaryLoader, CreateDestroy) {
  
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
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
TEST(BinaryLoader, CreateDestroyTwo) {

	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
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
		EXPECT_EQ(GetNumberOfFilesOfType(bl, "Test"), 2);

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
TEST(BinaryLoader, CreateFromFile)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
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
		EXPECT_EQ(GetNumberOfFilesOfType(bl, "Test"), 1);
		EXPECT_EQ(GetNumberOfFilesOfType(bl, "Korv"), 0);
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
TEST(BinaryLoader, CreateFromCallback)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
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
TEST(BinaryLoader, Defrag)
{
	std::error_code err;
	fs::remove("cd.dat", err);
	{
		auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
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