#include "testfixture.hh"
#include <dbfacade/update.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>

using namespace softeq;

struct SomeUpdate
{
    int id;
    std::string name;
    std::string time;
    int intval;
};

bool operator==(const SomeUpdate &lhs, const SomeUpdate &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.time == rhs.time && lhs.intval == rhs.intval;
}

template <>
const db::TableScheme db::buildTableScheme<SomeUpdate>()
{
    // clang-format off
    static const auto scheme =  db::TableScheme("UpdateTable",
        {
            {&SomeUpdate::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeUpdate::name, "name"},
            {&SomeUpdate::time, "time"},
            {&SomeUpdate::intval, "intval"}
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, UpdateWhere)
{
    using namespace db;

    TableGuard<SomeUpdate> someUpdateTable(_storage);

    _storage.execute(query::insert<SomeUpdate>({.id = 1, .name = "name1", .time = "2021-01-01", .intval = 1}));
    _storage.execute(query::insert<SomeUpdate>({.id = 2, .name = "name2", .time = "2021-01-02", .intval = 2}));
    _storage.execute(query::insert<SomeUpdate>({.id = 3, .name = "name3", .time = "2021-01-03", .intval = 3}));

    EXPECT_NO_THROW(
        _storage.execute(query::update<SomeUpdate>({.id = 1, .name = "NewName1", .time = "2022-01-01", .intval = 4})
                             .where(db::field(&SomeUpdate::id) == 1)));
    EXPECT_NO_THROW(
        _storage.execute(query::update<SomeUpdate>({.id = 2, .name = "NewName2", .time = "2022-01-02", .intval = 5})
                             .where(db::field(&SomeUpdate::id) == 2)));
    EXPECT_NO_THROW(
        _storage.execute(query::update<SomeUpdate>({.id = 3, .name = "NewName3", .time = "2022-01-03", .intval = 6})
                             .where(db::field(&SomeUpdate::id) == 3)));

    std::vector<SomeUpdate> data = _storage.receive(query::select<SomeUpdate>({}));

    EXPECT_EQ(data.at(0).name, "NewName1");
    EXPECT_EQ(data.at(0).time, "2022-01-01");
    EXPECT_EQ(data.at(0).intval, 4);

    EXPECT_EQ(data.at(1).name, "NewName2");
    EXPECT_EQ(data.at(1).time, "2022-01-02");
    EXPECT_EQ(data.at(1).intval, 5);

    EXPECT_EQ(data.at(2).name, "NewName3");
    EXPECT_EQ(data.at(2).time, "2022-01-03");
    EXPECT_EQ(data.at(2).intval, 6);
}

TEST_F(DBFacadeTestFixture, UpdateNonPrimary)
{
    using namespace db;

    TableGuard<SomeUpdate> someUpdateTable(_storage);

    _storage.execute(query::insert<SomeUpdate>({.id = 1, .name = "John", .time = "2021-01-01", .intval = 4}));
    _storage.execute(query::insert<SomeUpdate>({.id = 2, .name = "John", .time = "2021-01-02", .intval = 4}));
    _storage.execute(query::insert<SomeUpdate>({.id = 3, .name = "Jane", .time = "2021-01-03", .intval = 4}));

    // set time to "2022-01-03" to all Johns
    // UPDATE UpdateTable SET time = "2022-01-03" WHERE name = John
    SomeUpdate newData{};
    newData.time = "2022-01-03";

    EXPECT_THROW(_storage.execute(query::update({}, newData)), SqlException);

    EXPECT_NO_THROW(
        _storage.execute(query::update({&SomeUpdate::time}, newData).where(field(&SomeUpdate::name) == "John")));

    // make sure each John has time = 500
    std::vector<SomeUpdate> data =
        _storage.receive(query::select<SomeUpdate>({&SomeUpdate::time}).where(field(&SomeUpdate::name) == "John"));
    EXPECT_EQ(data.size(), 2);

    for (const auto &row : data)
    {
        EXPECT_EQ(row.time, "2022-01-03");
    }

    // ...and Jane still has time = "2021-01-03"
    data = _storage.receive(query::select<SomeUpdate>({&SomeUpdate::time}).where(field(&SomeUpdate::name) == "Jane"));
    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.at(0).time, "2021-01-03");
}
