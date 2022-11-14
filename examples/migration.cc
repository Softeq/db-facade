#include <gtest/gtest.h>

#include <common/migration/migration_task.hh>
#include <dbfacade/extensions/migration/migration.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/update.hh>
#include <dbfacade/select.hh>
#include <dbfacade/sqliteconnection.hh>

using namespace softeq;
namespace sql = db::query;

namespace
{
struct dbInfo
{
    std::string key;
    std::string value;
};

struct Student_v1
{
    int id;
    std::string name;
    int grade;
};

struct Student_v2
{
    int id;
    std::string fullName;
    std::string major;
};

using Student = Student_v2;

} // namespace

template <>
const db::TableScheme db::buildTableScheme<dbInfo>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("dbInfo",
        {
            {&dbInfo::key, "key"},
            {&dbInfo::value, "value"}
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<Student_v1>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
                                               {
                                                   {&Student_v1::id, "id", db::Cell::Flags::PRIMARY_KEY},
                                                   {&Student_v1::name, "name"},
                                                   {&Student_v1::grade, "grade"},
                                               }
        ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<Student_v2>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
                                               {
                                                   {&Student_v2::id, "id", db::Cell::Flags::PRIMARY_KEY},
                                                   {&Student_v2::fullName, "full_name"},
                                                   {&Student_v2::major, "major"},
                                               }
        ); // clang-format on
    return scheme;
}

class Database
{
    bool saveVersion(const common::migration::Version &value)
    {
        _storage.execute(sql::update<dbInfo>({"version", value.toString()}).where(db::field(&dbInfo::key) == "version"));
        return true;
    }

    common::migration::Version loadVersion()
    {
        std::vector<dbInfo> version =
            _storage.receive(sql::select<dbInfo>({}).where(db::field(&dbInfo::key) == "version"));
        common::migration::Version result;
        char c;
        std::stringstream ss(version[0].value);
        ss >> result.gen >> c >> result.major;
        return result;
    }

public:
    Database(common::migration::Manager &migration)
        : _migration(migration)
        , _storage(std::make_shared<db::SqliteConnection>("database.db"))
    {
        try
        {
            _storage.verifyScheme<dbInfo>();
        }
        catch(db::SqlException& ex)
        {
            std::cout << "Empty db: " <<  ex.what() << std::endl;
            _storage.execute(sql::createTable<dbInfo>());
            _storage.execute(sql::insert<dbInfo>({"version", "0.0"}));
        }
        catch(std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            throw ex;
        }

        _migration.setLoadVersionFunction(std::bind(&Database::loadVersion, this));
        _migration.setSaveVersionFunction(std::bind(&Database::saveVersion, this, std::placeholders::_1));

        // version 0.3 just creates the table student (version 1)
        _migration.addTask(common::migration::Task::UPtr(new db::migration::CreateTableTask<Student_v1>({0, 3}, _storage)));
        // version 1.2 alters table student to the version 2
        _migration.addTask(common::migration::Task::UPtr(new db::migration::AlterTableTask<Student_v1, Student_v2>(
            {1, 2}, _storage, {{&Student_v1::name, &Student_v2::fullName}})));
    }

    bool init()
    {
        return _migration.update().success;
    }

private:
    common::migration::Manager &_migration;
    db::Facade _storage;
};

int main()
{
//    migration::Version appVersion(0,1);
//    migration::Manager migration(appVersion);
    common::migration::Manager migration;
    Database db(migration);
    return db.init() ? EXIT_SUCCESS : EXIT_FAILURE;
}
