#include "testfixture.hh"
#include <dbfacade/remove.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>

using namespace softeq;

struct SomeRemove
{
    int id;
    std::string name;
    std::string time;
};

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeRemove>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("RemoveTable", 
        {
            {&SomeRemove::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeRemove::name, "name"},
            {&SomeRemove::time, "time"}
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, Remove)
{
    using namespace db;

    TableGuard<SomeRemove> someRemoveTable(_storage);

    _storage.execute(query::insert<SomeRemove>({.id = 1, .name = "name1", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeRemove>({.id = 2, .name = "name2", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeRemove>({.id = 3, .name = "name3", .time = "2021-01-03"}));

    std::vector<SomeRemove> data = _storage.receive(query::select<SomeRemove>({}));
    EXPECT_TRUE(data.size() == 3);

    EXPECT_NO_THROW(_storage.execute(query::remove<SomeRemove>().where(db::field(&SomeRemove::id) == 1)));
    EXPECT_NO_THROW(_storage.execute(query::remove<SomeRemove>().where(db::field(&SomeRemove::name) == "name3")));

    data = _storage.receive(query::select<SomeRemove>({}));

    EXPECT_TRUE(data.size() == 1);
    EXPECT_TRUE(data.front().name == "name2");
    EXPECT_TRUE(data.front().time == "2021-01-02");
}

