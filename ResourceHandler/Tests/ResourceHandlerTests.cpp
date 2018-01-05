#include "pch.h"
#include <filesystem>
#include "../Include/ResourceHandler/FileSystem_Interface.h"
#include "../Include/ResourceHandler/ResourceHandler_Interface.h"
#include <Utilz\ThreadPool.h>
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
			EXPECT_EQ(r.errornr, 0);

			Component asd{ 1,1,4 };
			auto size = sizeof(asd);
			r = CreateS_C(bl, "Comp1", "Comp", &asd, size);
			EXPECT_EQ(r.errornr, 0);

			DestroyLoader(bl);
		}


		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.errornr, 0);
			Utilz::ThreadPool tp(4);
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
				ResourceData<Component> comp;
				EXPECT_TRUE(re.GetData(comp.GetVoid()) & ResourceHandler::LoadStatus::SUCCESS);
				Component c2{ 1,1,4 };
				EXPECT_EQ(comp.Get(), c2);
			}
			ResourceHandler::Resource re("Comp1", "Comp");
			EXPECT_EQ(re.GetCheckInCount(), 0);
			EXPECT_EQ(re.GetReferenceCount(), 0);
			ResourceData<Component> comp;
			EXPECT_TRUE(re.GetData(comp.GetVoid()) & ResourceHandler::LoadStatus::SUCCESS);
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
			EXPECT_EQ(r.errornr, 0);

			Component asd{ 1,1,4 };
			auto size = sizeof(asd);
			r = CreateS_C(bl, "Comp1", "Comp", &asd, size);
			EXPECT_EQ(r.errornr, 0);

			DestroyLoader(bl);
		}


		{
			auto bl = CreateFileSystem(ResourceHandler::FileSystemType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data2.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r.errornr, 0);
			Utilz::ThreadPool tp(4);
			auto rh = CreateResourceHandler(bl, &tp);
			EXPECT_TRUE(rh);
			{
				ResourceHandler::Resource re("Comp1", "Comp");
				ResourceData<Component> comp;
				EXPECT_TRUE(re.GetData(comp.GetVoid()) & ResourceHandler::LoadStatus::SUCCESS);
				Component c2{ 1,1,4 };
				EXPECT_EQ(comp.Get(), c2);
				c2.y = 3;
				bl->Write("Comp1", "Comp", { &c2, sizeof(c2) });
			
			}
			ResourceHandler::Resource re("Comp1", "Comp", true);
			ResourceData<Component> comp;
			Component c2{ 1,3,4 };
			EXPECT_TRUE(re.GetData(comp.GetVoid()) & ResourceHandler::LoadStatus::SUCCESS);
			EXPECT_EQ(comp.Get(), c2);
			DestroyLoader(bl);
		}

		fs::remove("data2.dat", err);
	}
}
