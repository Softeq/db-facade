#include "testfixture.hh"
#include <dbfacade/insert.hh>
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

TEST_F(DBFacadeTestFixture, TransactionBasic)
{
    namespace sql = db::query;

    // create table
    TableGuard<Student> studentTable(_storage);

    // commit
    _storage.execTransaction([](db::Facade &storage) {
        storage.execute(sql::insert<Student>({0, "John"}));
        storage.execute(sql::insert<Student>({1, "Jane"}));
        return true;
    });

    std::vector<Student> data = _storage.receive(sql::select<Student>({}));
    EXPECT_EQ(data.size(), 2);

    // rollback
    _storage.execTransaction([](db::Facade &storage) {
        storage.execute(sql::insert<Student>({2, "Jack"}));
        storage.execute(sql::insert<Student>({3, "Jean"}));

        std::vector<Student> data = storage.receive(sql::select<Student>({}));
        EXPECT_EQ(data.size(), 4);
        return false;
    });

    data = _storage.receive(sql::select<Student>({}));
    EXPECT_EQ(data.size(), 2);
}

TEST_F(DBFacadeTestFixture, TransactionSimple)
{
    namespace sql = db::query;

    TableGuard<Student> studentTable(_storage);

    // TODO: find a way to make sure these actions were really executed inside a transaction
    _storage.execTransaction(sql::insert<Student>({0, "John"}), sql::insert<Student>({1, "Jack"}),
                             sql::insert<Student>({2, "Jack"}), sql::insert<Student>({3, "Jean"}));

    std::vector<Student> data = _storage.receive(sql::select<Student>({}));
    EXPECT_EQ(data.size(), 4);
}

