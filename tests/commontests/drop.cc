#include "testfixture.hh"
#include <dbfacade/createtable.hh>
#include <dbfacade/drop.hh>
#include <dbfacade/select.hh>

using namespace softeq;

namespace
{
struct Student
{
    int id;
    std::string name;
};
} // namespace

template <>
const db::TableScheme db::buildTableScheme<Student>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&Student::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&Student::name, "name"}
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, DropBasic)
{
    using namespace db;

    // create table
    EXPECT_NO_THROW(_storage.execute(db::query::createTable<Student>()));

    // check the table exists
    std::vector<Student> data = _storage.receive(query::select<Student>({}));
    EXPECT_EQ(data.size(), 0);

    // drop table
    _storage.execute(db::query::drop<Student>());

    // check the table is dropped
    EXPECT_THROW(data = _storage.receive(query::select<Student>({})), SqlException);
}

