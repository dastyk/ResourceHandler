#include "stdafx.h"
#include "CppUnitTest.h"
#include <ResourceHandler_Interface.h>
#include <memory>

#ifdef _DEBUG
#pragma comment(lib, "ResourceHandlerD.lib")
#else
#pragma comment(lib, "ResourceHandler.lib")
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ResourceCopyTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: Your test code here

			auto rh = std::make_unique(ResourceHandler::CreateResourceHandler());
			Assert::IsNotNull(rh.get(), L"Could not create Resource Handler");
		}

	};
}