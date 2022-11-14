#include <gtest/gtest.h>
#include <commontests/testfixture.hh>
#include <mysql/mysqlconnection.hh>

using namespace softeq;

DBFacadeTestFixture::DBFacadeTestFixture()
    : _connection(new softeq::db::mysql::MySqlConnection("127.0.0.1", 3306, "user", "secret", "db"))
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
