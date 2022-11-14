#include <gtest/gtest.h>
#include <commontests/testfixture.hh>
#include <dbfacade/sqliteconnection.hh>

using namespace softeq;

DBFacadeTestFixture::DBFacadeTestFixture()
    : _connection(new db::SqliteConnection(":memory:"))
    , _storage(_connection)
{
}

void DBFacadeTestFixture::SetUp()
{
}

void DBFacadeTestFixture::TearDown()
{
}

int main(int argc, char *argv[])
{
    return RunTests(argc, argv);
}

