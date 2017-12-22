#include "stdafx.h"
#include "CppUnitTest.h"
#include <ResourceHandler\ResourceHandler_Interface.h>
#include <ResourceHandler\Loader_Interface.h>
#include <filesystem>
#include <Utilz\ThreadPool.h>
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
				auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
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

				result = bl->Read("TestFile", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFile");
				Assert::IsNotNull(data.data, L"TestFile data was nullptr");
				Assert::IsTrue(data.size == 9, L"TestFile size was not 9");
				Assert::IsTrue(std::string((char*)data.data, data.size) == "Test File", L"Content in TestFile was not 'Test File'");
				result = bl->Read("TestFile2", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFile2");
				Assert::IsNotNull(data.data, L"TestFile2 data was nullptr");
				Assert::IsTrue(data.size == 10, L"TestFile2 size was not 10");
				Assert::IsTrue(std::string((char*)data.data, data.size) == "Test File2", L"Content in TestFile was not 'Test File2'");

				result = bl->Destroy("TestFile", "Test");
				Assert::IsTrue(result == 0, L"Could not destroy TestFile");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 10, L"Total file size not 10, after destroy that is next to last");
				result = bl->Defrag();
				Assert::IsTrue(result == 0, L"Defrag failed");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 10, L"Total file size not 10, after defrag");

				result = bl->Read("TestFile2", "Test", data);
				Assert::IsTrue(result == 0, L"Could not read TestFile2 after defrag");
				Assert::IsNotNull(data.data, L"TestFile2 data was nullptr");
				Assert::IsTrue(data.size == 10, L"TestFile2 size was not 10");

				auto asd = std::string((char*)data.data, data.size);
				Assert::IsTrue(std::string((char*)data.data, data.size) == "Test File2", L"Content in TestFile2 was not 'Test File2'");

				result = bl->Destroy("TestFile2", "Test");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 0, L"Total file size not 10, after destroying last file");
				Assert::IsTrue(result == 0, L"Could not destroy TestFile2");
				Assert::IsTrue(bl->GetNumberOfFiles() == 0, L"Num files not changed");

				result = bl->Defrag();
				Assert::IsTrue(result == 0, L"Defrag failed");
				Assert::IsTrue(bl->GetTotalSizeOfAllFiles() == 0, L"Total file size not 0, after defrag");

				bl->Shutdown();
			}
			fs::remove("basictest.dat");
		}
		TEST_METHOD(BasicBinaryLoaderRunTimeTest)
		{
			fs::remove("BasicRuntimeTest.dat");
			{
				{
					auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
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
					auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
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
					auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
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
					auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
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
				auto bl = ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary);
				bl->Init("rhtest.dat", ResourceHandler::Mode::READ);
				Utilz::ThreadPool tp(4);

				auto rh = std::make_unique(ResourceHandler::CreateResourceHandler(bl, &tp));
				Assert::IsNotNull(rh.get(), L"Could not create Resource Handler");

				auto resource1 = rh->LoadResource("TestFile", "Test");

				Assert::IsTrue(resource1.Status() == ResourceHandler::LoadStatus::SUCCESS, L"TestFile could not be loaded");

				resource1.GetData();

				bl->Shutdown();
				delete bl;

			}
			fs::remove("rhtest.dat");
		}
	};
}