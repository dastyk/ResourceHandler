#include "stdafx.h"
#include "CppUnitTest.h"
#include <ResourceHandler\ResourceHandler_Interface.h>
#include <ResourceHandler\Loader_Interface.h>
#include <filesystem>
#include <Utilz\ThreadPool.h>
#include <fstream>
namespace fs = std::experimental::filesystem;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
namespace BasicResourceHandlerTest
{		
	TEST_CLASS(ResourceHandlerTests)
	{
	public:
		TEST_METHOD(ThreadPoolTest)
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
				Assert::IsTrue(someInt == 1000);
			}
			//Testing adding a lambda
			{
				auto someLambda = []() {return 5; };
				auto future = tp.Enqueue(someLambda);
				Assert::IsTrue(future.get() == 5);
			}

			//Testing a regular function.
			{
				auto future = tp.Enqueue(Chungus, 10, 15);	
				Assert::IsTrue(future.get() == Chungus(10, 15));
			}

			// Testing methods
			{
				X x(10);
				auto ret = tp.Enqueue(&x, &X::Foo, 20);
				Assert::IsTrue(ret.get() == 20 + x.i);
				Assert::IsTrue(x.Do(&tp));
			}
		}

		TEST_METHOD(BasicBinaryLoaderEditTest)
		{
			fs::remove("basictest.dat");
			{
				auto bl = std::make_unique(CreateLoader(ResourceHandler::LoaderType::Binary));
				Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
				auto result = bl->Init("basictest.dat", ResourceHandler::Mode::EDIT);
				Assert::IsTrue(result == 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

				char buf[] = { "Test File" };
				ResourceData data;
				data.data = buf;
				data.size = 9;
				result = bl->Create("TestFile", "Test", data);
				Assert::IsTrue(result == 0 || result == 1, L"Could not create TestFile");
				char buf2[] = { "Test File2" };
				data.data = buf2;
				data.size = 10;
				result = bl->Create("TestFile2", "Test", data);
				Assert::IsTrue(result == 0 || result == 1, L"Could not create TestFile2");
				Assert::IsTrue(bl->GetNumberOfFiles() == 2, L"More than two files");
				Assert::IsTrue(bl->GetNumberOfTypes() == 1, L"More than one type");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 19, L"Total file size not 19");

				result = bl->Exist("TestFile", "Test");
				Assert::IsTrue(result == 1, L"TestFile did not exist");
				result = bl->Exist("TestFile2", "Test");
				Assert::IsTrue(result == 1, L"TestFile2 did not exist");

				data.size = 0;
				result = bl->GetSizeOfFile("TestFile", "Test", data.size);
				Assert::IsTrue(result == 0, L"Could not get size of TestFile");
				Assert::IsTrue(data.size == 9, L"Size of TestFile was not 9");
				data.data = operator new(data.size);

				result = bl->Read("TestFile", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFile");
				Assert::IsTrue(std::string((char*)data.data, data.size) == "Test File", L"Content in TestFile was not 'Test File'");
				operator delete(data.data);
				
				data.size = 0;
				result = bl->GetSizeOfFile("TestFile2", "Test", data.size);
				Assert::IsTrue(result == 0, L"Could not get size of TestFile2");
				Assert::IsTrue(data.size == 10, L"Size of TestFile2 was not 10");
				data.data = operator new(data.size);

				result = bl->Read("TestFile2", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFile2");
				Assert::IsTrue(std::string((char*)data.data, data.size) == "Test File2", L"Content in TestFile was not 'Test File2'");
				operator delete(data.data);

				bl->Shutdown();
				result = bl->Init("basictest.dat", ResourceHandler::Mode::EDIT);
				Assert::IsTrue(result == 0, L"Could not reinit loader");

				std::vector<ResourceHandler::File> files;
				result = bl->GetFilesOfType("Test", files);
				Assert::IsTrue(result == 0, L"Could not get filesoftype");
				Assert::IsTrue(files.size() == 2, L"Files size not 2");
				Assert::IsTrue(files[0].guid == "TestFile", L"Files not TestFile");
				Assert::IsTrue(files[0].type == "Test", L"Filestype not Test");
				Assert::IsTrue(files[0].guid_str == "TestFile", L"Filesstr not TestFile");
				Assert::IsTrue(files[0].type_str == "Test", L"Filestypestr not Test");
				Assert::IsTrue(files[1].guid == "TestFile2", L"Files not TestFile2");
				Assert::IsTrue(files[1].type == "Test", L"Filestype not Test");
				Assert::IsTrue(files[1].guid_str == "TestFile2", L"Filesstr not TestFile2");
				Assert::IsTrue(files[1].type_str == "Test", L"Filestypestr not Test");

				ResourceHandler::FILE_C* fc = new ResourceHandler::FILE_C[2];
				uint32_t nF = 2;
				GetFiles_C(bl.get(), fc, nF);

				

				result = bl->Destroy("TestFile", "Test");
				Assert::IsTrue(result == 0, L"Could not destroy TestFile");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 10, L"Total file size not 20, after destroy that is next to last");
				result = bl->Defrag();
				Assert::IsTrue(result == 0, L"Defrag failed");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 10, L"Total file size not 10, after defrag");

				result = bl->Read("TestFile2", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFile2 after defrag");
				auto asd = std::string((char*)data.data, data.size);
				Assert::IsTrue(std::string((char*)data.data, data.size) == "Test File2", L"Content in TestFile2 was not 'Test File2'");

				//result = bl->Destroy("TestFile2", "Test");
				//Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 0, L"Total file size not 10, after destroying TestFile2 file");
				//Assert::IsTrue(result == 0, L"Could not destroy TestFile2");
				//Assert::IsTrue(bl->GetNumberOfFiles() == 0, L"Num files not changed");

				//result = bl->Defrag();
				//Assert::IsTrue(result == 0, L"Defrag failed");
				//Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 0, L"Total file size not 0, after defrag");


				result = bl->CreateFromFile("test.txt", "TestFileTxt", "Test");
				Assert::IsTrue(result == 0, L"Could not create from file");
				Assert::IsTrue(bl->GetNumberOfFiles() == 2, L"Num files not 2 before reinit");

				bl->Shutdown();
				bl.reset();
				bl = std::make_unique(CreateLoader(ResourceHandler::LoaderType::Binary));
				result = bl->Init("basictest.dat", ResourceHandler::Mode::EDIT);
				Assert::IsTrue(result == 0, L"Could not reinit");
				Assert::IsTrue(bl->GetNumberOfFiles() == 2, L"Numfiles not 2 after reinit");
				data.data = operator new(data.size);
				data.size = 4;
				result = bl->Read("TestFileTxt", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFileTxt");
				Assert::IsTrue(std::string((char*)data.data, data.size) == "test", L"Content in test.txt was not 'test'");
				operator delete(data.data);


				bl->Shutdown();
			}
			fs::remove("basictest.dat");
		}
		TEST_METHOD(BasicBinaryLoaderRunTimeTest)
		{
			fs::remove("BasicRuntimeTest.dat");
			{
				{
					auto bl = std::make_unique(CreateLoader(ResourceHandler::LoaderType::Binary));
					Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
					auto result = bl->Init("BasicRuntimeTest.dat", ResourceHandler::Mode::READ);
					Assert::IsTrue(result == 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

					char buf[] = { "Test File" };
					ResourceData data;
					data.data = buf;
					data.size = 9;
					result = bl->Create("TestFile", "Test", data);
					Assert::IsTrue(result == -1, L"Could create TestFile");
				}
				{
					auto bl = std::make_unique(CreateLoader(ResourceHandler::LoaderType::Binary));
					Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
					auto result = bl->Init("BasicRuntimeTest.dat", ResourceHandler::Mode::EDIT);
					Assert::IsTrue(result == 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

					char buf[] = { "Test File" };
					ResourceData data;
					data.data = buf;
					data.size = 9;
					result = bl->Create("TestFile", "Test", data);
					Assert::IsTrue(result == 0 || result == 1, L"Could not create TestFile");
				}

				{
					auto bl = std::make_unique(CreateLoader(ResourceHandler::LoaderType::Binary));
					Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
					auto result = bl->Init("BasicRuntimeTest.dat", ResourceHandler::Mode::READ);
					Assert::IsTrue(result == 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

					result = bl->Destroy("TestFile", "Test");
					Assert::IsTrue(result != 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

				}
			}
			fs::remove("BasicRuntimeTest.dat");

		}
		TEST_METHOD(BasicResourceHandlerTest)
		{
			fs::remove("rhtest.dat");
			{
				{
					auto bl = std::make_unique(CreateLoader(ResourceHandler::LoaderType::Binary));
					auto result = bl->Init("rhtest.dat", ResourceHandler::Mode::EDIT);
					char buf[] = { "Test File" };
					ResourceData data;
					data.data = buf;
					data.size = 9;
					result = bl->Create("TestFile", "Test", data);
					char buf2[] = { "Test File2" };
					data.data = buf2;
					data.size = 10;
					result = bl->Create("TestFile2", "Test", data);

				}
				auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
				bl->Init("rhtest.dat", ResourceHandler::Mode::READ);
				Utilz::ThreadPool tp(4);

				auto rh = std::make_unique(ResourceHandler::CreateResourceHandler(bl, &tp));
				Assert::IsNotNull(rh.get(), L"Could not create Resource Handler");

				ResourceHandler::Resource resource1("TestFile", "Test");
				rh->LoadResource(resource1); 

				Assert::IsTrue(01u == rh->GetReferenceCount(resource1), L"Refcount not 0", LINE_INFO());
				Assert::IsTrue(0u == resource1.GetCheckInCount(), L"CheckInCount not 0", LINE_INFO());

				rh->CheckIn(resource1);

				Assert::IsTrue(1u == rh->GetReferenceCount(resource1), L"Refcount not 1", LINE_INFO());
				Assert::IsTrue(1u == resource1.GetCheckInCount(), L"CheckInCount not 1", LINE_INFO());

				rh->CheckIn(resource1);

				Assert::IsTrue(1u == rh->GetReferenceCount(resource1), L"Refcount not 1", LINE_INFO());
				Assert::IsTrue(2u == resource1.GetCheckInCount(), L"CheckInCount not 2", LINE_INFO());

				rh->CheckOut(resource1);

				Assert::IsTrue(1u == rh->GetReferenceCount(resource1), L"Refcount not 1", LINE_INFO());
				Assert::IsTrue(1u == resource1.GetCheckInCount(), L"CheckInCount not 1", LINE_INFO());

				auto r2 = resource1;

				rh->CheckIn(resource1);

				Assert::IsTrue(2u == rh->GetReferenceCount(resource1), L"Refcount not 2", LINE_INFO());
				Assert::IsTrue(1u == resource1.GetCheckInCount(), L"CheckInCount not 1", LINE_INFO());
				Assert::IsTrue(1u == r2.GetCheckInCount(), L"CheckInCount not 1", LINE_INFO());

				bl->Shutdown();
				delete bl;

			}
			fs::remove("rhtest.dat");
		}
	};
}