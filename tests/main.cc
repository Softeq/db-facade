#include <gtest/gtest.h>
#include <dbfacade/version.hh>

using namespace softeq;

TEST(DBFacade, Library)
{
    std::string ver;
    EXPECT_NO_THROW(ver = db::getVersion());
    EXPECT_TRUE(ver.size() > 0);

    std::string components;
    EXPECT_NO_THROW(components = db::getComponents());
}

class TestFixture : public ::testing::Environment
{
protected:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    // gtest takes ownership of the TestEnvironment ptr - we don't delete it.
    ::testing::AddGlobalTestEnvironment(new TestFixture());
    return RUN_ALL_TESTS();
}
