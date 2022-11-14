#include "testfixture.hh"
#include <dbfacade/select.hh>
#include <dbfacade/insert.hh>

using namespace softeq;

namespace
{
struct OldEmployee
{
    int id;
    std::string name;
    int grade;
};

struct Employee
{
    int id;
    std::string name;
    int department_id;
    int64_t project_id;
};
} // namespace

template <>
const db::TableScheme db::buildTableScheme<OldEmployee>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("old_employee",
        {
            {&OldEmployee::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&OldEmployee::name, "name"},
            {&OldEmployee::grade, "grade"},
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<Employee>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("employee",
        {
            {&Employee::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&Employee::name, "name"},
            {&Employee::department_id, "department_id", db::Cell::Flags::DEFAULT, 10},
            {&Employee::project_id, "project_id"} // nullable field
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, CreateTableBasic)
{
    namespace sql = db::query;

    // create table
    TableGuard<Employee> employeeTable(_storage);

    // check the table exists
    std::vector<Employee> data = _storage.receive(sql::select<Employee>({}));
    EXPECT_EQ(data.size(), 0);
}

TEST_F(DBFacadeTestFixture, CopyTable)
{
    namespace sql = db::query;

    // create table
    TableGuard<OldEmployee> oldEmployeeTable(_storage);

    // fill some values
    _storage.execute(sql::insert<OldEmployee>({.id = 0, .name = "John", .grade = 80}));
    _storage.execute(sql::insert<OldEmployee>({.id = 1, .name = "Jane", .grade = 70}));
    _storage.execute(sql::insert<OldEmployee>({.id = 2, .name = "Jack", .grade = 60}));

    // create new table with previous one's data
    _storage.execute(sql::createTable<Employee>()
                         .asSelect<OldEmployee>()
                         .where(db::field(&OldEmployee::name) != "Jack")
                         .orderBy(&OldEmployee::id));

    // check if the data is copied
    std::vector<Employee> data = _storage.receive(sql::select<Employee>({&Employee::department_id}));
    EXPECT_EQ(data.size(), 2);
    EXPECT_EQ(data.at(0).department_id, 10);

    // this is not exactly a test but a demonstration that we cannot currently deserialize a nullable field
    EXPECT_THROW(data = _storage.receive(sql::select<Employee>({&Employee::project_id})), std::exception);

    // there should not be 'grade' in new table
    EXPECT_THROW(data = _storage.receive(sql::select<Employee>({&OldEmployee::grade})), db::SqlException);

    _storage.execute(db::query::drop<Employee>());
}

namespace
{
struct EmptyColumnName
{
    std::string empty;
};

struct TwoPKs
{
    int id1;
    int id2;
};
} // namespace

template <>
const db::TableScheme db::buildTableScheme<EmptyColumnName>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("EmptyColumnName",
        {
            {&EmptyColumnName::empty, ""},
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<TwoPKs>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("two_pk",
        {
            {&TwoPKs::id1, "id1", db::Cell::Flags::PRIMARY_KEY},
            {&TwoPKs::id2, "id2", db::Cell::Flags::PRIMARY_KEY},
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, VariousTables)
{
    namespace sql = db::query;

    // Column is unnamed exception
    EXPECT_THROW(_storage.execute(sql::createTable<EmptyColumnName>()), db::SqlException);

    // Two pks exception
    EXPECT_THROW(_storage.execute(sql::createTable<TwoPKs>()), db::SqlException);
}
