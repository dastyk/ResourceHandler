#include "pch.h"
#include <filesystem>
#include "../Include/ResourceHandler/FileSystem_Interface.h"
#include "../Include/ResourceHandler/ResourceHandler_Interface.h"
#include <ThreadPool.h>
namespace fs = std::experimental::filesystem;

TEST(ResourceHandler, BasicLoad)
{
	{
		std::error_code err;
		fs::remove("data.dat", err);
		struct Component
		{
			int x, y, z;
			bool operator==(const Component& o)const
			{
				return x == o.x && y == o.y && z == o.z;
			}
		};
		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.hash, "Success"_hash);

			Component asd{ 1,1,4 };
			auto size = sizeof(asd);
			r = CreateS_C(bl, "Comp1", "Comp", &asd, size);
			EXPECT_EQ(r.hash, "Success"_hash);

			DestroyLoader(bl);
		}


		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.hash, "Success"_hash);
			Utilities::ThreadPool tp(4);
			auto rh = CreateResourceHandler(bl, &tp);
			EXPECT_TRUE(rh);
			{
				ResourceHandler::Resource re("Comp1", "Comp");
				EXPECT_EQ(re.GetCheckInCount(), 0);
				EXPECT_EQ(re.GetReferenceCount(), 0);
				re.CheckIn();
				EXPECT_EQ(re.GetCheckInCount(), 1);
				EXPECT_EQ(re.GetReferenceCount(), 1);
				re.CheckIn();
				EXPECT_EQ(re.GetCheckInCount(), 2);
				EXPECT_EQ(re.GetReferenceCount(), 1);
				ResourceHandler::ResourceData<Component> comp(re);
				EXPECT_NO_THROW(comp.Get());
				Component c2{ 1,1,4 };
				//EXPECT_EQ(*comp, c2);
			}
			ResourceHandler::Resource re("Comp1", "Comp");
			EXPECT_EQ(re.GetCheckInCount(), 0);
			EXPECT_EQ(re.GetReferenceCount(), 0);
			ResourceHandler::ResourceData<Component> comp(re);
			EXPECT_NO_THROW(comp.Get());
			DestroyResourceHandler(rh);
			DestroyLoader(bl);
		}


		fs::remove("data.dat", err);

	}
	
}

TEST(ResourceHandler, Invalidate)
{
	{
		std::error_code err;
		fs::remove("data2.dat", err); 
		struct Component
		{
			int x, y, z;
			bool operator==(const Component& o)const
			{
				return x == o.x && y == o.y && z == o.z;
			}
		};
		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data2.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.hash, "Success"_hash);

			Component asd{ 1,1,4 };
			auto size = sizeof(asd);
			r = CreateS_C(bl, "Comp1", "Comp", &asd, size);
			EXPECT_EQ(r.hash, "Success"_hash);

			DestroyLoader(bl);
		}


		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data2.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.hash, "Success"_hash);
			Utilities::ThreadPool tp(4);
			auto rh = CreateResourceHandler(bl, &tp);
			EXPECT_TRUE(rh);
			{
				ResourceHandler::ResourceData<Component> comp("Comp1", "Comp");
				EXPECT_NO_THROW(comp.Get());
				Component c2{ 1,1,4 };
				EXPECT_EQ(*comp, c2);
				c2.y = 3;
				bl->Write("Comp1", "Comp", { &c2, sizeof(c2) });
			
			}
			ResourceHandler::Resource re("Comp1", "Comp", true);
			ResourceHandler::ResourceData<Component> comp(re);
			Component c2{ 1,3,4 };
			EXPECT_NO_THROW(comp.Get());
			EXPECT_EQ(comp.Get(), c2);
			DestroyResourceHandler(rh);
			DestroyLoader(bl);
		}

		fs::remove("data2.dat", err);
	}


}
#include <fstream>

TEST(ResourceHandler, PasstroughTest)
{
	{
		std::error_code err;
		fs::remove("data.dat", err);
		fs::remove("Comp.pat", err);
		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.hash, "Success"_hash);
			Utilities::ThreadPool tp(4);
			auto rh = CreateResourceHandler(bl, &tp);
			EXPECT_TRUE(rh);

			r = rh->Initialize();
			EXPECT_EQ(r.hash, "Success"_hash);

			uint32_t testInt = 1;
			r = CreateS_C(bl, "Comp1", "Comp", &testInt, sizeof(testInt));
			EXPECT_EQ(r.hash, "Success"_hash);

			r = ResourceHandler_CreateType(rh, "Comp", "TestPassthrough.dll");
			EXPECT_EQ(r.hash, "Success"_hash);

			DestroyResourceHandler(rh);
			DestroyLoader(bl);
		}


		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.hash, "Success"_hash);
			Utilities::ThreadPool tp(4);
			auto rh = CreateResourceHandler(bl, &tp);
			EXPECT_TRUE(rh);
			
			r = rh->Initialize();
			EXPECT_EQ(r.hash, "Success"_hash);

			ResourceHandler::ResourceData<uint32_t> comp("Comp1", "Comp");
			EXPECT_NO_THROW(comp.Get());


			EXPECT_EQ(*comp, 2);

			DestroyResourceHandler(rh);
			DestroyLoader(bl);
		}

		fs::remove("Comp.pat", err);
		fs::remove("data.dat", err);

	}

}