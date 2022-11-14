#include <gtest/gtest.h>
#include <migration/migration.hh>
#include <dbfacade/select.hh>
#include <dbfacade/sqliteconnection.hh>

using namespace softeq;
namespace sql = db::query;

namespace
{
struct Student
{
    int id;
    std::string name;
    int grade;
};

struct UnivStudent
{
    int id;
    std::string fullName;
    std::string major;
};

} // namespace

template <>
const db::TableScheme db::buildTableScheme<Student>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&Student::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&Student::name, "name"},
            {&Student::grade, "grade"},
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<UnivStudent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&UnivStudent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&UnivStudent::fullName, "full_name"},
            {&UnivStudent::major, "major"},
        }
    ); // clang-format on
    return scheme;
}

class DBMigrationTasks : public testing::Test
{
    bool saveVersion(const common::migration::Version &value)
    {
        _version = value;
        return true;
    }

    common::migration::Version loadVersion()
    {
        return _version;
    }

public:
    DBMigrationTasks()
        : _connection(new db::SqliteConnection(":memory:"))
        , _storage(_connection)
    {
    }

protected:
    void SetUp() override
    {
        _version = common::migration::Version();
        _mgr.reset(new common::migration::Manager);
        _mgr->setLoadVersionFunction(std::bind(&DBMigrationTasks::loadVersion, this));
        _mgr->setSaveVersionFunction(std::bind(&DBMigrationTasks::saveVersion, this, std::placeholders::_1));
    }

protected:
    db::Connection::SPtr _connection;
    db::Facade _storage;
    std::unique_ptr<common::migration::Manager> _mgr;
    common::migration::Version _version;
};

TEST_F(DBMigrationTasks, CreateDrop)
{
    _mgr->addTask(common::migration::Task::UPtr(new db::migration::CreateTableTask<Student>({0, 1}, _storage)));
    _mgr->update();

    std::vector<Student> data;
    EXPECT_NO_THROW(data = _storage.receive(sql::select<Student>({})));
    EXPECT_EQ(data.size(), 0);

    _mgr->addTask(common::migration::Task::UPtr(new db::migration::DeleteTableTask<Student>({1, 2}, _storage)));
    _mgr->update();

    EXPECT_THROW(data = _storage.receive(sql::select<Student>({})), db::SqlException);
}

TEST_F(DBMigrationTasks, Alter)
{
    _mgr->addTask(common::migration::Task::UPtr(new db::migration::CreateTableTask<Student>({0, 1}, _storage)));
    _mgr->addTask(common::migration::Task::UPtr(new db::migration::AlterTableTask<Student, UnivStudent>(
        {1, 2}, _storage, {{&Student::name, &UnivStudent::fullName}})));
    EXPECT_TRUE(_mgr->update().success);

    std::vector<UnivStudent> data;
    data = _storage.receive(sql::select<UnivStudent>({})); // select * from student;
    // we should have 'major' field
    EXPECT_NO_THROW(data = _storage.receive(sql::select<UnivStudent>({&UnivStudent::major})));
    // and should not have 'grade' field
    EXPECT_THROW(data = _storage.receive(sql::select<UnivStudent>({&Student::grade})), db::SqlException);
    EXPECT_EQ(data.size(), 0);
}
