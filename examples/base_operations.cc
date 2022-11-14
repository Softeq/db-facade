#include <dbfacade/sqliteconnection.hh>
#include <dbfacade/createtable.hh>
#include <dbfacade/facade.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>
#include <dbfacade/update.hh>
#include <dbfacade/remove.hh>
#include <dbfacade/drop.hh>

// 1. To work with the database, you need to create a table structure
struct SomeTable
{
    int id;
    std::string name;
    std::string time;
};

// 2. Redefine operator== for all table structure fields
bool operator==(const SomeTable &lhs, const SomeTable &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.time == rhs.time;
}

// 3. Define mapping rules between a C++ structure and a database table. It is used to emulate reflection in C++.
template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeTable>()
{
    // clang-format off
    static const auto scheme = softeq::db::TableScheme("SomeTable",
        {
            {&SomeTable::id, "id", softeq::db::Cell::Flags::PRIMARY_KEY},
            {&SomeTable::name, "name"},
            {&SomeTable::time, "time"}
        }
    ); // clang-format on
    return scheme;
}

int main()
{
    using namespace softeq;
    using namespace db;

    // 4. Create connection according to the rules described in the constructor
    Connection::SPtr connection(new SqliteConnection(":memory:"));

    // 5. Pass the created connection to the facade
    Facade storage(connection);

    // 6. Executing a query to create a new table in a database
    storage.execute(query::createTable<SomeTable>());

    //-----------------------------------------------------------------------------------

    // Checking an already existing table in the database and matching the structure

    // PRAGMA table_info('SomeTable');
    storage.verifyScheme<SomeTable>();

    //-----------------------------------------------------------------------------------

    // Insert data into database

    SomeTable row1 = SomeTable{.id = 1, .name = "name1", .time = "2022-01-01"};
    SomeTable row2 = SomeTable{.id = 2, .name = "name2", .time = "2022-01-02"};
    SomeTable row3 = SomeTable{.id = 3, .name = "name3", .time = "2022-01-03"};

    // INSERT INTO SomeTable VALUES (1, 'name1', '2022-01-01');
    storage.execute(query::insert(row1));
    storage.execute(query::insert(row2));
    storage.execute(query::insert(row3));

    //-----------------------------------------------------------------------------------

    // Select and retrieving data from the database

    // SELECT * FROM SomeTable;
    std::vector<SomeTable> data = storage.receive(query::select<SomeTable>({}));

    // SELECT SomeTable.id FROM SomeTable WHERE (SomeTable.id = 1);
    data = storage.receive(query::select<SomeTable>({&SomeTable::id}).where(db::field(&SomeTable::id) == 1));

    // SELECT SomeTable.name FROM SomeTable WHERE (SomeTable.name = 'name2');
    data = storage.receive(query::select<SomeTable>({&SomeTable::name}).where(db::field(&SomeTable::name) == "name2"));

    // SELECT SomeTable.id, SomeTable.time FROM SomeTable WHERE (SomeTable.name = 'name3') ORDER BY SomeTable.time ASC;
    data = storage.receive(query::select<SomeTable>({&SomeTable::id, &SomeTable::time})
                               .where(field(&SomeTable::name) == "name3")
                               .orderBy(&SomeTable::time));

    //-----------------------------------------------------------------------------------

    // Update data in the database

    // UPDATE SomeTable SET name = 'NewName1', time = '2022-01-01' WHERE (SomeTable.id = 1);
    storage.execute(query::update<SomeTable>({.id = 1, .name = "NewName1", .time = "2022-01-01"})
                        .where(db::field(&SomeTable::id) == 1));

    // UPDATE SomeTable SET name = 'NewName2', time = '2022-01-02' WHERE (SomeTable.id = 2);
    storage.execute(query::update<SomeTable>({.id = 2, .name = "NewName2", .time = "2022-01-02"})
                        .where(db::field(&SomeTable::id) == 2));

    // UPDATE SomeTable SET name = 'NewName3', time = '2022-01-03' WHERE (SomeTable.id = 3);
    storage.execute(query::update<SomeTable>({.id = 3, .name = "NewName3", .time = "2022-01-03"})
                        .where(db::field(&SomeTable::id) == 3));

    //-----------------------------------------------------------------------------------

    // Remove data from database

    // DELETE FROM SomeTable WHERE (SomeTable.id = 1);
    storage.execute(query::remove<SomeTable>().where(db::field(&SomeTable::id) == 1));

    // DELETE FROM SomeTable WHERE (SomeTable.name = 'name3');
    storage.execute(query::remove<SomeTable>().where(db::field(&SomeTable::name) == "name3"));

    //-----------------------------------------------------------------------------------

    // Drop database

    // DROP TABLE IF EXISTS SomeTable;
    storage.execute(db::query::drop<SomeTable>());
}
