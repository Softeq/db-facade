#include "testfixture.hh"
#include "dbfacade/select.hh"
#include "dbfacade/insert.hh"

using namespace softeq;

struct SomeSelect
{
    int id;
    std::string name;
    std::string time;
};

bool operator==(const SomeSelect &lhs, const SomeSelect &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.time == rhs.time;
}

template <>
const db::TableScheme db::buildTableScheme<SomeSelect>()
{
    // clang-format off
    static const auto scheme =  db::TableScheme("SelectTable",
        {
            {&SomeSelect::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeSelect::name, "name"},
            {&SomeSelect::time, "time"}
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, SelectBase)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    _storage.execute(query::insert<SomeSelect>({.id = 1, .name = "name1", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeSelect>({.id = 2, .name = "name2", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeSelect>({.id = 3, .name = "name3", .time = "2021-01-03"}));

    std::vector<SomeSelect> data;

    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id}).where(db::field(&SomeSelect::id) == 1));
    EXPECT_TRUE(data.front().id == 1);

    data =
        _storage.receive(query::select<SomeSelect>({&SomeSelect::name}).where(db::field(&SomeSelect::name) == "name2"));
    EXPECT_TRUE(data.front().name == "name2");

    data = _storage.receive(
        query::select<SomeSelect>({&SomeSelect::time}).where(db::field(&SomeSelect::time) == "2021-01-03"));
    EXPECT_TRUE(data.front().time == "2021-01-03");
}

TEST_F(DBFacadeTestFixture, SelectOrderBy)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    _storage.execute(query::insert<SomeSelect>({.id = 1, .name = "John", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeSelect>({.id = 2, .name = "Jane", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeSelect>({.id = 3, .name = "Jane", .time = "2021-01-03"}));

    std::vector<SomeSelect> data;

    // SELECT id, time FROM SomeSelect WHERE (name = 'Jane') ORDER BY time;
    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::time})
                                .where(field(&SomeSelect::name) == "Jane")
                                .orderBy(&SomeSelect::time));

    EXPECT_EQ(data.size(), 2);
    EXPECT_TRUE(data.at(0).time == "2021-01-02");
    EXPECT_TRUE(data.at(1).time == "2021-01-03");

    // SELECT id, time FROM SomeSelect WHERE (name = 'Jane') ORDER BY id DESC;
    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::time})
                                .where(field(&SomeSelect::name) == "Jane")
                                .orderBy({&SomeSelect::id, OrderBy::DESC}));

    EXPECT_EQ(data.size(), 2);
    EXPECT_TRUE(data.at(0).time == "2021-01-03");
    EXPECT_TRUE(data.at(1).time == "2021-01-02");
}

TEST_F(DBFacadeTestFixture, SelectComplex)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    _storage.execute(query::insert<SomeSelect>({.id = 1, .name = "name1", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeSelect>({.id = 2, .name = "name2", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeSelect>({.id = 3, .name = "name3", .time = "2021-01-03"}));

    std::vector<SomeSelect> data;

    EXPECT_NO_THROW(data = _storage.receive(
                        query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                            .where(db::field(&SomeSelect::name) != "name1" and
                                   (db::field(&SomeSelect::id) == 1 or db::field(&SomeSelect::id).between(2, 3)))));

    EXPECT_TRUE(data.size() == 2);

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(
            query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                .where(db::field(&SomeSelect::time) < "2021-01-03" and (db::field(&SomeSelect::time) > "2021-01-01"))));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.front().time, "2021-01-02");

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::time) <= "2021-01-03" and
                                           (db::field(&SomeSelect::time) >= "2021-01-01"))));

    EXPECT_EQ(data.size(), 3);
    EXPECT_TRUE(data.at(0).time <= data.at(1).time);
}

TEST_F(DBFacadeTestFixture, SelectNoData)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    std::vector<SomeSelect> data;

    EXPECT_NO_THROW(data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::time})
                                                .where(field(&SomeSelect::name) == "Jane")
                                                .orderBy({&SomeSelect::id, OrderBy::DESC})));

    EXPECT_EQ(data.size(), 0);
}

TEST_F(DBFacadeTestFixture, SelectLikeTest)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    _storage.execute(query::insert<SomeSelect>({.id = 1, .name = "name1", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeSelect>({.id = 2, .name = "name2", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeSelect>({.id = 3, .name = "name3", .time = "2021-01-03"}));

    std::vector<SomeSelect> data;

    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::name).like("n%2"))));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.front().name, "name2");

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::name).like("____1"))));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.front().id, 1);

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::name).like(""))));

    EXPECT_EQ(data.size(), 0);
}

TEST_F(DBFacadeTestFixture, SelectInTest)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    _storage.execute(query::insert<SomeSelect>({.id = 1, .name = "name1", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeSelect>({.id = 2, .name = "name2", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeSelect>({.id = 3, .name = "name3", .time = "2021-01-03"}));

    std::vector<SomeSelect> data;

    EXPECT_NO_THROW(data = _storage.receive(
                        query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                            .where(db::field(&SomeSelect::time).in({"2021-03-01", "2021-02-01", "2021-01-01"}))));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.front().id, 1);

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::time).in({"", "2021-05-01", "2021-01-02"}))));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.front().id, 2);

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::time).in({"2021-01-01", "2021-01-02", 100}))));

    EXPECT_EQ(data.size(), 2);

    data.clear();
    EXPECT_NO_THROW(
        data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                    .where(db::field(&SomeSelect::id).in({0, 1, 3}))));

    EXPECT_EQ(data.size(), 2);

    data.clear();
    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id, &SomeSelect::name, &SomeSelect::time})
                                .where(db::field(&SomeSelect::name).in({"", "name", "name3"})));

    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.front().id, 3);
}

TEST_F(DBFacadeTestFixture, SelectLimits)
{
    using namespace db;

    TableGuard<SomeSelect> someSelectTable(_storage);

    _storage.execute(query::insert<SomeSelect>({.id = 1, .name = "name1", .time = "2021-01-01"}));
    _storage.execute(query::insert<SomeSelect>({.id = 2, .name = "name2", .time = "2021-01-02"}));
    _storage.execute(query::insert<SomeSelect>({.id = 3, .name = "name3", .time = "2021-01-03"}));

    std::vector<SomeSelect> data;

    // just limit
    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id}).limit(2));
    EXPECT_EQ(data.size(), 2);

    // limit + offset
    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id}).limit(1).offset(1));
    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(data.at(0).id, 2);

    // just offset
    data = _storage.receive(query::select<SomeSelect>({&SomeSelect::id}).offset(1).orderBy(&SomeSelect::id));
    EXPECT_EQ(data.size(), 2);
    EXPECT_TRUE(data.at(0).id == 2 && data.at(1).id == 3);
}
