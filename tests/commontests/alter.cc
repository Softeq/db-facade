#include "testfixture.hh"
#include <dbfacade/alter.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>

using namespace softeq;

// Old table

struct OldStudent
{
    int id;
    std::string name;
};

template <>
const db::TableScheme db::buildTableScheme<OldStudent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("old_student",
        {
            {&OldStudent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&OldStudent::name, "name"}
        }
    ); // clang-format on
    return scheme;
}

// New table

struct NewStudent
{
    int id;
    std::string fullName;
    std::unique_ptr<std::string> major;
    int grade;
};

template <>
const db::TableScheme db::buildTableScheme<NewStudent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&NewStudent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&NewStudent::fullName, "full_name"},
            {&NewStudent::major, "major", db::columntypes::Nullable},
            {&NewStudent::grade, "grade", db::Cell::Flags::DEFAULT, 50}
        }
    ); // clang-format on
    return scheme;
}

struct DropRenameStudent
{
    int studentId;
};

template <>
const db::TableScheme db::buildTableScheme<DropRenameStudent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&DropRenameStudent::studentId, "student_id", db::Cell::Flags::PRIMARY_KEY},
        }
    ); // clang-format on
    return scheme;
}

struct NonExistingStudent
{
    int id;
    std::string name;
    std::string nonExistingFullName;
    int grade;
};

template <>
const db::TableScheme db::buildTableScheme<NonExistingStudent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&NonExistingStudent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&NonExistingStudent::nonExistingFullName, "nonExistingFullName"},
            {&NonExistingStudent::name, "non_exitsing_name"},
            {&NonExistingStudent::grade, "grade", db::Cell::Flags::DEFAULT, 50}
        }
    ); // clang-format on
    return scheme;
}

struct DoubleKeyStudent
{
    int id;
    std::string name;
    int second_id;
};

template <>
const db::TableScheme db::buildTableScheme<DoubleKeyStudent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&DoubleKeyStudent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&DoubleKeyStudent::name, "name"},
            {&DoubleKeyStudent::second_id, "second_key", db::Cell::Flags::PRIMARY_KEY}
            
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, AlterBasic)
{
    namespace sql = db::query;

    TableGuard<OldStudent> oldStudentTable(_storage);

    _storage.execute(sql::insert<OldStudent>({.id = 1, .name = "John"}));
    _storage.execute(sql::insert<OldStudent>({.id = 2, .name = "Jane"}));

    EXPECT_THROW(_storage.execute(
                     sql::alterScheme<OldStudent, NewStudent>().renamingCell(&NewStudent::fullName, &OldStudent::name)),
                 db::SqlException);

    EXPECT_THROW(_storage.execute(sql::alterScheme<OldStudent, NewStudent>().renamingCell(
                     &OldStudent::name, &NonExistingStudent::nonExistingFullName)),
                 db::SqlException);

    _storage.execute(sql::alterScheme<OldStudent, NewStudent>().renamingCell(&OldStudent::name, &NewStudent::fullName));

    std::vector<NewStudent> data = _storage.receive(
        sql::select<NewStudent>({&NewStudent::fullName, &NewStudent::grade}).orderBy({&NewStudent::id}));

    EXPECT_EQ(data.size(), 2);
    // make sure name column was renamed, not dropped
    EXPECT_EQ(data.at(0).fullName, "John");
    EXPECT_EQ(data.at(0).grade + data.at(1).grade, 50 + 50);
    EXPECT_THROW(data = _storage.receive(sql::select<NewStudent>({&OldStudent::name})), db::SqlException);

    _storage.execute(db::query::drop<NewStudent>());
}

TEST_F(DBFacadeTestFixture, AlterDropColumn)
{
    // Some DBMS (like Sqlite) cannot drop columns. We need to test this flow
    namespace sql = db::query;

    TableGuard<OldStudent> oldStudentTable(_storage);

    _storage.execute(sql::insert<OldStudent>({.id = 1, .name = "John"}));
    _storage.execute(sql::insert<OldStudent>({.id = 2, .name = "Jane"}));

    _storage.execute(sql::alterScheme<OldStudent, NewStudent>()); // drop name and add fullName

    std::vector<NewStudent> data = _storage.receive(sql::select<NewStudent>({&NewStudent::id}));

    EXPECT_EQ(data.size(), 2);

    _storage.execute(db::query::drop<NewStudent>());
}

TEST_F(DBFacadeTestFixture, AlterDropRename)
{
    namespace sql = db::query;

    TableGuard<OldStudent> oldStudentTable(_storage);

    _storage.execute(sql::insert<OldStudent>({.id = 11, .name = "John"}));
    _storage.execute(sql::insert<OldStudent>({.id = 22, .name = "Jane"}));

    _storage.execute(
        sql::alterScheme<OldStudent, DropRenameStudent>().renamingCell(&OldStudent::id, &DropRenameStudent::studentId));

    std::vector<DropRenameStudent> data =
        _storage.receive(sql::select<DropRenameStudent>({&DropRenameStudent::studentId}));

    EXPECT_EQ(data.size(), 2);
    // make sure column was renamed, not dropped
    EXPECT_EQ(data.at(0).studentId + data.at(1).studentId, 11 + 22);

    _storage.execute(db::query::drop<NewStudent>());
}
