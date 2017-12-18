#include "stdafx.h"
#include "CppUnitTest.h"
#include "../includes/ResourceHandler_Interface.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#ifdef _DEBUG
#pragma comment(lib, "ResourceHandlerD.lib")
#else
#pragma comment(lib, "ResourceHandler.lib")
#endif
namespace BasicResourceHandlerTest
{		
	TEST_CLASS(UnitTest1)
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
		TEST_METHOD(LoadResourceTest)
		{
			// TODO: Your test code here
			auto rh = std::make_unique(ResourceHandler::CreateResourceHandler());
			Assert::IsNotNull(rh.get(), L"Could not create Resource Handler");

			auto r = rh->LoadResource("TestResource.txt");
			auto status = r.GetStatus();
			Assert::IsTrue(status == ResourceHandler::LoadStatus::SUCCESS, L"");
		}
	};
}