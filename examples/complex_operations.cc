#include "dbfacade/sqliteconnection.hh"
#include "dbfacade/createtable.hh"
#include "dbfacade/facade.hh"
#include "dbfacade/insert.hh"
#include "dbfacade/select.hh"
#include "dbfacade/drop.hh"
#include "dbfacade/alter.hh"
#include "dbfacade/alter.hh"
#include "dbfacade/remove.hh"

// 1. To work with the database, you need to create a table structure
struct Student
{
    int id;
    std::string name;
    std::string time;
};

// 2. Redefine operator== for all table structure fields
bool operator==(const Student &lhs, const Student &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.time == rhs.time;
}

// 3. Define mapping rules between a C++ structure and a database table. It is used to emulate reflection in C++.
template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<Student>()
{
    // clang-format off
    static const auto scheme = softeq::db::TableScheme("Student",
        {
            {&Student::id, "id", softeq::db::Cell::Flags::PRIMARY_KEY},
            {&Student::name, "name"},
            {&Student::time, "time"}
        }
    ); // clang-format on
    return scheme;
}

struct NewStudent
{
    int id;
    std::string fullName;
    std::string major;
    int grade;
};

bool operator==(const NewStudent &lhs, const NewStudent &rhs)
{
    return lhs.id == rhs.id && lhs.fullName == rhs.fullName && lhs.major == rhs.major && lhs.grade == rhs.grade;
}

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<NewStudent>()
{
    // clang-format off
    static const auto scheme = softeq::db::TableScheme("NewStudent",
        {
            {&NewStudent::id, "id", softeq::db::Cell::Flags::PRIMARY_KEY},
            {&NewStudent::fullName, "full_name"},
            {&NewStudent::major, "major"},
            {&NewStudent::grade, "grade", softeq::db::Cell::Flags::DEFAULT, 50}
        }
    ); // clang-format on
    return scheme;
}

struct Marks
{
    int student_id;
    int mark;
    int task;
};

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<Marks>()
{
    // clang-format off
    static const auto scheme =  softeq::db::TableScheme("Marks",
        {
            {&Marks::student_id, "student_id"},
            {&Marks::mark, "mark"},
            {&Marks::task, "task"}
        }
    ); // clang-format on
    return scheme;
}

struct SomeCascadeParent
{
    int id;
    int sec_id;
};

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeCascadeParent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("ParentTable", 
        {
            {&SomeCascadeParent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeCascadeParent::sec_id, "sec_id"}
        }
    ); // clang-format on
    return scheme;
}

struct SomeCascadeChild
{
    int id;
    int ref_id;
};

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeCascadeChild>()
{
    using namespace softeq::db::constraints;

    // clang-format off
    static const auto scheme = db::TableScheme("ChildTable", 
        {
            {&SomeCascadeChild::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeCascadeChild::ref_id, "ref_id"}
        },
        {
            makeConstraint<ForeignKeyConstraint>(&SomeCascadeChild::ref_id, &SomeCascadeParent::id, 
                            addCascade(CascadeTrigger::OnUpdate, CascadeAction::Cascade),
                            addCascade(CascadeTrigger::OnDelete, CascadeAction::Cascade))
        //     // "r_ChildTable",
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
    storage.execute(query::createTable<Student>());

    //-----------------------------------------------------------------------------------

    // Transaction

    // The instruction is used to batch read and write operations so that the database
    // system can ensure data consistency.

    // BEGIN TRANSACTION;
    // INSERT INTO Student VALUES (1, 'name1', '2022-01-01');
    // INSERT INTO Student VALUES (2, 'name2', '2022-01-02');
    // INSERT INTO Student VALUES (3, 'name3', '2022-01-03');
    // COMMIT;
    storage.execTransaction([](db::Facade &storage) {
        storage.execute(query::insert<Student>({.id = 1, .name = "name1", .time = "2022-01-01"}));
        storage.execute(query::insert<Student>({.id = 2, .name = "name2", .time = "2022-01-02"}));
        storage.execute(query::insert<Student>({.id = 3, .name = "name3", .time = "2022-01-03"}));
        return true;
    });

    // Rollback Transaction

    // If one of the queries in the query group executed by the transaction fails,
    // all previously executed queries are rolled back.

    // BEGIN TRANSACTION;
    // INSERT INTO Student VALUES (4, 'name4', '2022-01-04');
    // INSERT INTO Student VALUES (5, 'name5', '2022-01-05');
    // ROLLBACK;
    storage.execTransaction([](db::Facade &storage) {
        storage.execute(query::insert<Student>({.id = 4, .name = "name4", .time = "2022-01-04"}));
        storage.execute(query::insert<Student>({.id = 5, .name = "name5", .time = "2022-01-05"}));
        return false;
    });

    //-----------------------------------------------------------------------------------

    // Join.

    // Designed to provide a selection of data from two tables and include this data in one result set

    storage.execute(query::createTable<Marks>());
    storage.execute(query::insert<Marks>({.student_id = 1, .mark = 87, .task = 1001}));
    storage.execute(query::insert<Marks>({.student_id = 2, .mark = 97, .task = 1002}));
    storage.execute(query::insert<Marks>({.student_id = 2, .mark = 90, .task = 1003}));

    std::vector<std::tuple<Student, Marks>> data;

    // SELECT * FROM Marks JOIN Student ON (Student.id = Marks.student_id);
    data = storage.receive(query::select<Marks>({}).join<Student>(field(&Student::id) == field(&Marks::student_id)));

    // SELECT Student.name, Marks.mark FROM Student
    // JOIN Marks ON (Student.id = Marks.student_id) WHERE (Student.name = 'name2');
    data = storage.receive(query::select<Student>({&Student::name, &Marks::mark})
                               .join<Marks>(field(&Student::id) == field(&Marks::student_id))
                               .where(field(&Student::name) == "name2"));

    //-----------------------------------------------------------------------------------

    // Alter.

    // The operator is used to add, modify or remove/delete columns in a table. Also used to rename a table.

    // If you need some column to be renamed, you need to explicitly specify this using renamingCell
    // Under the hood, it works like this: we count the difference between the schemes, that is,
    // we find which columns are new and which have disappeared. Adding new ones, deleting missing ones.
    // There is no way to determine simply from the schemas that the column needs to be renamed,
    // and not the old one to be deleted, but the new one to be added, so you must specify it explicitly

    // BEGIN TRANSACTION;
    // CREATE TABLE tmp_Student AS SELECT id, name AS full_name, CAST(50 AS INTEGER) AS grade,
    // CAST(NULL AS TEXT) AS major FROM Student;
    // DROP TABLE Student; ALTER TABLE tmp_Student RENAME TO NewStudent;
    // COMMIT;
    storage.execute(query::alterScheme<Student, NewStudent>().renamingCell(&Student::name, &NewStudent::fullName));

    //-----------------------------------------------------------------------------------

    // Cascade delete.

    // A foreign key with cascade delete means that if a record in the parent table is deleted,
    // then the corresponding records in the child table will automatically be deleted.

    storage.execute(query::createTable<SomeCascadeParent>());
    storage.execute(query::insert<SomeCascadeParent>({.id = 1, .sec_id = 10}));
    storage.execute(query::insert<SomeCascadeParent>({.id = 2, .sec_id = 20}));
    storage.execute(query::insert<SomeCascadeParent>({.id = 3, .sec_id = 30}));

    // CREATE TABLE IF NOT EXISTS ChildTable(id INTEGER PRIMARY KEY, ref_id INTEGER, FOREIGN KEY ( ref_id )
    // REFERENCES ParentTable ( id ) ON UPDATE CASCADE ON DELETE CASCADE);
    storage.execute(query::createTable<SomeCascadeChild>());
    storage.execute(query::insert<SomeCascadeChild>({.id = 2, .ref_id = 2}));

    // DELETE FROM ParentTable WHERE (ParentTable.id = 2);
    storage.execute(query::remove<SomeCascadeParent>().where(db::field(&SomeCascadeParent::id) == 2));

    //-----------------------------------------------------------------------------------

    // Drop databases

    storage.execute(db::query::drop<Marks>());
    storage.execute(db::query::drop<Student>());
    storage.execute(db::query::drop<NewStudent>());
    storage.execute(db::query::drop<SomeCascadeParent>());
    storage.execute(db::query::drop<SomeCascadeChild>());
}
