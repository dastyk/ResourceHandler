#include "stdafx.h"
#include "CppUnitTest.h"
#include "../includes/ResourceHandler_Interface.h"
#include <Loader_Interface.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#ifdef _DEBUG
#pragma comment(lib, "ResourceHandlerD.lib")
#else
#pragma comment(lib, "ResourceHandler.lib")
#endif

namespace BasicResourceHandlerTest
{		
	TEST_CLASS(ResourceHandlerTests)
	{
	public:
		TEST_METHOD(CreateResourceHandlerTest)
		{
			// TODO: Your test code here
			auto rh = std::make_unique(ResourceHandler::CreateResourceHandler());
			Assert::IsNotNull(rh.get(), L"Could not create Resource Handler");

			rh.reset();
			Assert::IsNull(rh.get(), L"rh is not null");
		}


		TEST_METHOD(BasicBinaryLoaderEditTest)
		{
			auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
			Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
			auto result = bl->Init(ResourceHandler::Mode::EDIT);
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
			result = bl->Destroy("TestFile2", "Test");
			Assert::IsTrue(result == 0, L"Could not destroy TestFile2");
			bl->Shutdown();
		}
		TEST_METHOD(BasicBinaryLoaderRunTimeTest)
		{
			{
				auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
				Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
				auto result = bl->Init(ResourceHandler::Mode::READ);
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
				auto result = bl->Init(ResourceHandler::Mode::EDIT);
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
				auto result = bl->Init(ResourceHandler::Mode::READ);
				Assert::IsTrue(result == 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

				result = bl->Destroy("TestFile", "Test");
				Assert::IsTrue(result != 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());

			}

		}
		TEST_METHOD(StressBinaryLoaderTest)
		{
			auto bl = std::make_unique(ResourceHandler::CreateLoader(ResourceHandler::LoaderType::Binary));
			Assert::IsNotNull(bl.get(), L"Could not create BinaryLoader");
			auto result = bl->Init(ResourceHandler::Mode::EDIT);
			Assert::IsTrue(result == 0, (L"Could not Init BinaryLoader, Error: " + std::to_wstring(result)).c_str());


		}
		//TEST_METHOD(LoadResourceTest)
		//{
		//	// TODO: Your test code here
		//	auto rh = std::make_unique(ResourceHandler::CreateResourceHandler());
		//	Assert::IsNotNull(rh.get(), L"Could not create Resource Handler");

		//	auto r2 = rh->LoadResource("TestResource", "Text");
		//	Assert::IsTrue(r2.GUID() == "TestResource.txt", L"GUID not correct");
		//	Assert::IsTrue(r2.Status() == ResourceHandler::LoadStatus::SUCCESS, L"Could not load resource");

		//	auto r = rh->LoadResource("Not Found", "None");
		//	Assert::IsTrue(r.Status() == ResourceHandler::LoadStatus::NOT_FOUND);


		//
		//}
	};
}