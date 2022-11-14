#include "testfixture.hh"
#include <dbfacade/join.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>

using namespace softeq;

// Main table

struct Student
{
    int id;
    std::string name;
    std::string time;
};

template <>
const db::TableScheme db::buildTableScheme<Student>()
{
    // clang-format off
    static const auto scheme =  db::TableScheme("student",
        {
            {&Student::id, "id"},
            {&Student::name, "name"},
            {&Student::time, "time"}
        }
    ); // clang-format on
    return scheme;
}

// Secondary table (to join with)

struct Marks
{
    int student_id;
    int mark;
    int task;
};

template <>
const db::TableScheme db::buildTableScheme<Marks>()
{
    // clang-format off
    static const auto scheme =  db::TableScheme("marks",
        {
            {&Marks::student_id, "student_id"},
            {&Marks::mark, "mark"},
            {&Marks::task, "task"}
        }
    ); // clang-format on
    return scheme;
}

// One more table to join with

struct Publications
{
    int task;
    std::string ref;
};

template <>
const db::TableScheme db::buildTableScheme<Publications>()
{
    // clang-format off
    static const auto scheme =  db::TableScheme("publications",
        {
            {&Publications::task, "task"},
            {&Publications::ref, "ref"}
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, JoinBasic)
{
    namespace qb = db::query;

    TableGuard<Marks> marksTable(_storage);
    TableGuard<Student> studentTable(_storage);

    _storage.execute(qb::insert<Student>({.id = 1, .name = "John", .time = "2021-01-01"}));
    _storage.execute(qb::insert<Student>({.id = 2, .name = "Jane", .time = "2021-01-02"}));

    _storage.execute(qb::insert<Marks>({.student_id = 1, .mark = 87, .task = 1001}));
    _storage.execute(qb::insert<Marks>({.student_id = 2, .mark = 97, .task = 1002}));
    _storage.execute(qb::insert<Marks>({.student_id = 2, .mark = 90, .task = 1003}));

    std::vector<std::tuple<Student, Marks>> data;

    // select student.name, marks.mark from student join marks on student.id = marks.student_id where student.name =
    // 'Jane'
   data = _storage.receive(db::query::select<Student>({&Student::name, &Marks::mark})
                                .join<Marks>(db::field(&Student::id) == db::field(&Marks::student_id))
                                .where(db::field(&Student::name) == "Jane"));

    EXPECT_EQ(data.size(), 2);
    EXPECT_EQ(std::get<1>(data.at(0)).mark + std::get<1>(data.at(1)).mark, 90 + 97);
    EXPECT_EQ(std::get<0>(data.at(0)).name, "Jane");

    // select * from marks join student on student.id = marks.student_id
    data = _storage.receive(
        db::query::select<Marks>({}).join<Student>(db::field(&Student::id) == db::field(&Marks::student_id)));

    EXPECT_EQ(data.size(), 3);

    // we will get SqlException (unknown cell) if we do not specify all required tables in the result
    // e.g. we need Marks int the tuple because we require &Marks::mark
    EXPECT_THROW(std::vector<std::tuple<Student>> missingData =
                     _storage.receive(db::query::select<Marks>({&Marks::mark})
                                          .join<Student>(db::field(&Student::id) == db::field(&Marks::student_id))),
                 db::SqlException);
}

TEST_F(DBFacadeTestFixture, JoinThreeTables)
{
    namespace qb = db::query;

    TableGuard<Marks> marksTable(_storage);
    TableGuard<Student> studentTable(_storage);
    TableGuard<Publications> publicationsTable(_storage);

    _storage.execute(qb::insert<Student>({.id = 1, .name = "John", .time = "2021-01-01"}));
    _storage.execute(qb::insert<Student>({.id = 2, .name = "Jane", .time = "2021-01-02"}));

    _storage.execute(qb::insert<Marks>({.student_id = 1, .mark = 87, .task = 1001}));
    _storage.execute(qb::insert<Marks>({.student_id = 2, .mark = 97, .task = 1002}));
    _storage.execute(qb::insert<Marks>({.student_id = 2, .mark = 90, .task = 1003}));

    _storage.execute(db::query::insert<Publications>({.task = 1001, .ref = "Science 2013; 342: 577."}));

    // select student.name, marks.task, publications.ref from student
    // join marks on student.id = marks.student_id
    // join publications on marks.task = publications.task
    // where student.name = 'John'
    std::vector<std::tuple<Student, Marks, Publications>> data =
        _storage.receive(db::query::select<Student>({&Student::name, &Marks::task, &Publications::ref})
                             .join<Marks>(db::field(&Student::id) == db::field(&Marks::student_id))
                             .join<Publications>(db::field(&Marks::task) == db::field(&Publications::task))
                             .where(db::field(&Student::name) == "John"));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(std::get<0>(data.at(0)).name, "John");
    EXPECT_EQ(std::get<1>(data.at(0)).task, 1001);
    EXPECT_EQ(std::get<2>(data.at(0)).ref, "Science 2013; 342: 577.");
}
