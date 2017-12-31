#include "pch.h"
#include <filesystem>
#include "../Include/ResourceHandler/Loader_Interface.h"
#include "../Include/ResourceHandler/ResourceHandler_Interface.h"
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
			auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r, 0);

			Component asd{ 1,1,4 };
			auto size = sizeof(asd);
			r = CreateS_C(bl, "Comp1", "Comp", &asd, size);
			EXPECT_EQ(r, 0);

			DestroyLoader(bl);
		}


		{
			auto bl = CreateLoader(ResourceHandler::LoaderType::Binary);
			EXPECT_TRUE(bl);

			auto r = InitLoader_C(bl, "data.dat", ResourceHandler::Mode::EDIT);
			EXPECT_EQ(r, 0);
			Utilz::ThreadPool tp(4);
			auto rh = CreateResourceHandler(bl, &tp);
			EXPECT_TRUE(rh);
			{
				ResourceHandler::Resource re("Comp1", "Comp");
				re.Load();
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

			DestroyLoader(bl);
		}


		fs::remove("data.dat", err);

	}
}