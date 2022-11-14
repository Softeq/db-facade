#ifndef SOFTEQ_DBFACADE_TESTCONFIG_H_
#define SOFTEQ_DBFACADE_TESTCONFIG_H_

#include <gtest/gtest.h>
#include <dbfacade/facade.hh>
#include <dbfacade/connection.hh>
#include <dbfacade/createtable.hh>
#include <dbfacade/drop.hh>

namespace softeq
{
/*!
    \brief Class for running test cases. The target module must define a constructor with the necessary database
    connection. It is also necessary to define the functions SetUp() and TearDown() are empty or with logic if needed
*/
class DBFacadeTestFixture : public testing::Test
{
public:
    DBFacadeTestFixture();

protected:
    void SetUp() override;
    void TearDown() override;

protected:
    db::Connection::SPtr _connection;
    db::Facade _storage;
};

/*!
    \brief Function to run tests in the target module.
    Should be called from main with 'argc' and 'argv' parameters passing
*/
int RunTests(int argc, char *argv[]);

/*!
    \brief The class is engaged in automatic creation and deletion of a table in the database. This is necessary in
    order to ensure that the table is destroyed if the test is interrupted as a result of an error.
*/
template <typename Struct>
class TableGuard final
{
public:
    TableGuard(db::Facade &storage)
        : _storage(storage)
    {
        _storage.execute(db::query::createTable<Struct>());
    }
    ~TableGuard()
    {
        _storage.execute(db::query::drop<Struct>());
    }

private:
    db::Facade &_storage;
};
} // namespace softeq
#endif

