#include "testfixture.hh"
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>

using namespace softeq;

struct SomeInsert
{
    int id;
    std::string name;
    int time;
};

bool operator==(const SomeInsert &lhs, const SomeInsert &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.time == rhs.time;
}

template <>
const db::TableScheme db::buildTableScheme<SomeInsert>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("InsertTable",
        {
            {&SomeInsert::id, "id", db::Cell::PRIMARY_KEY | db::Cell::AUTOINCREMENT},
            {&SomeInsert::name, "name"},
            {&SomeInsert::time, "time", db::Cell::DEFAULT, 20220101}
        }
    ); // clang-format on
    return scheme;
}

struct InsertWithNulls
{
    int id;
    std::unique_ptr<std::string> text;
};

template <>
const db::TableScheme db::buildTableScheme<InsertWithNulls>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("InsertWithNulls",
        {
            {&InsertWithNulls::id, "id", db::Cell::PRIMARY_KEY | db::Cell::AUTOINCREMENT},
            {&InsertWithNulls::text, "text", db::columntypes::Nullable}, // explicitly nullable
        }
    ); // clang-format on
    return scheme;
}

struct InsertUnique
{
    int code;
};

template <>
const db::TableScheme db::buildTableScheme<InsertUnique>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("InsertUnique",
        {
            {&InsertUnique::code, "code", db::Cell::UNIQUE},
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, InsertBasic)
{
    using namespace db;

    TableGuard<SomeInsert> someInsertTable(_storage);

    auto testRow1 = SomeInsert{.id = 1, .name = "name1", .time = 20210101};
    auto testRow2 = SomeInsert{.id = 2, .name = "name2", .time = 20210102};
    auto testRow3 = SomeInsert{.id = 3, .name = "name3", .time = 20210103};

    std::vector<SomeInsert> expectedResult;
    expectedResult.push_back(testRow1);
    expectedResult.push_back(testRow2);
    expectedResult.push_back(testRow3);

    EXPECT_NO_THROW(_storage.execute(query::insert(testRow1)));
    EXPECT_NO_THROW(_storage.execute(query::insert(testRow2)));
    EXPECT_NO_THROW(_storage.execute(query::insert(testRow3)));

    std::vector<SomeInsert> data = _storage.receive(query::select<SomeInsert>({}));
    EXPECT_EQ(data, expectedResult);
}

TEST_F(DBFacadeTestFixture, InsertSpecificFields)
{
    // TODO: make id auto increment and test it
    using namespace db;

    TableGuard<SomeInsert> someInsertTable(_storage);

    std::vector<SomeInsert> insertData{{.id = 1, .name = "name1", .time = 20210101},
                                       {.id = 2, .name = "name2", .time = 20210102},
                                       {.id = 3, .name = "name3", .time = 20210103}};

    for (const auto &row : insertData)
    {
        // do not insert time, it should be set to 20220101 (default)
        _storage.execute(query::insert({&SomeInsert::id, &SomeInsert::name}, row));
    }

    std::vector<SomeInsert> data = _storage.receive(query::select<SomeInsert>({}).orderBy(&SomeInsert::id));
    EXPECT_EQ(data.size(), insertData.size());
    for (size_t i = 0; i < data.size(); ++i)
    {
        EXPECT_EQ(data.at(i).id, insertData.at(i).id);
        EXPECT_EQ(data.at(i).name, insertData.at(i).name);
        EXPECT_EQ(data.at(i).time, 20220101);
    }
}

TEST_F(DBFacadeTestFixture, InsertAutoincrement)
{
    using namespace db;

    TableGuard<SomeInsert> someInsertTable(_storage);

    std::vector<SomeInsert> insertData{{.id = -1, .name = "name1", .time = 20210101},
                                       {.id = -1, .name = "name2", .time = 20210102},
                                       {.id = -1, .name = "name3", .time = 20210103}};

    for (const auto &row : insertData)
    {
        _storage.execute(query::insert({&SomeInsert::time, &SomeInsert::name}, row));
    }

    std::vector<SomeInsert> data = _storage.receive(query::select<SomeInsert>({}));
    EXPECT_EQ(data.size(), insertData.size());
    for (size_t i = 1; i < data.size(); ++i)
    {
        EXPECT_GT(data.at(i).id, data.at(i - 1).id);
    }
}

TEST_F(DBFacadeTestFixture, InsertError)
{
    using namespace db;

    TableGuard<SomeInsert> someInsertTable(_storage);

    auto testRow1 = SomeInsert{.id = 1, .name = "name1", .time = 20210101};
    auto testRow2 = SomeInsert{.id = 2, .name = "name2", .time = 20210102};
    auto testRow3 = SomeInsert{.id = 3, .name = "name3", .time = 20210103};

    std::vector<SomeInsert> expectedResult;
    expectedResult.push_back(testRow1);
    expectedResult.push_back(testRow2);
    expectedResult.push_back(testRow3);

    EXPECT_NO_THROW(_storage.execute(query::insert<SomeInsert>({.id = 1, .name = "name1", .time = 20210101})));
    EXPECT_NO_THROW(_storage.execute(query::insert<SomeInsert>({.id = 2, .name = "name2", .time = 20210102})));
    EXPECT_NO_THROW(_storage.execute(query::insert<SomeInsert>({.id = 3, .name = "name3", .time = 20210103})));

    std::vector<SomeInsert> data = _storage.receive(query::select<SomeInsert>({}));
    EXPECT_EQ(data, expectedResult);
}

TEST_F(DBFacadeTestFixture, InsertWithNulls)
{
    using namespace db;

    TableGuard<InsertWithNulls> insertTable(_storage);

    _storage.execute(
        query::insert(InsertWithNulls{.id = 1, .text = std::unique_ptr<std::string>(new std::string("John"))}));
    _storage.execute(query::insert(InsertWithNulls{.id = 2, .text = nullptr}));
    _storage.execute(query::insert({&InsertWithNulls::id}, InsertWithNulls{.id = 3, .text = nullptr}));

    std::vector<InsertWithNulls> data =
        _storage.receive(query::select<InsertWithNulls>({}).orderBy(&InsertWithNulls::id));
    EXPECT_EQ(*data.at(0).text, "John");
    EXPECT_FALSE(data.at(1).text);
    EXPECT_FALSE(data.at(2).text);
}

TEST_F(DBFacadeTestFixture, InsertUnique)
{
    using namespace db;

    TableGuard<InsertUnique> someInsertTable(_storage);

    _storage.execute(query::insert(InsertUnique{1}));

    // insert another - should not throw
    EXPECT_NO_THROW(_storage.execute(query::insert(InsertUnique{2})));

    // insert existing value, should throw
    EXPECT_THROW(_storage.execute(query::insert(InsertUnique{1})), SqlException);
}
