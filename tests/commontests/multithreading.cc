#include "testfixture.hh"
#include <dbfacade/sqliteconnection.hh>
#include <dbfacade/createtable.hh>
#include <dbfacade/select.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/drop.hh>
#include <thread>
#include <mutex>

using namespace softeq;

struct SomeMtRecord
{
    int id;
    int attr1;
    int attr2;
};

bool operator==(const SomeMtRecord &lhs, const SomeMtRecord &rhs)
{
    return lhs.id == rhs.id && lhs.attr1 == rhs.attr1 && lhs.attr2 == rhs.attr2;
}

bool isConsistent(const SomeMtRecord &record)
{
    return record.id == record.attr1 && record.attr1 == record.attr2;
}

template <>
const db::TableScheme db::buildTableScheme<SomeMtRecord>()
{
    // clang-format off
    static const auto scheme =  db::TableScheme("MtTable",
        {
            {&SomeMtRecord::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeMtRecord::attr1, "attr1"},
            {&SomeMtRecord::attr2, "attr2"}
        }
    ); // clang-format on
    return scheme;
}

class SQLiteMultithreading : public testing::Test
{
};

template <typename T>
class LockedVar
{
private:
    T _data;
    std::mutex _m;
    bool _fetched = true;

public:
    LockedVar(){};
    LockedVar(const T &t)
        : _data(t){};

    bool fetch(T &out)
    {
        const std::lock_guard<std::mutex> lock(_m);
        if (!_fetched)
        {
            out = _data;
            _fetched = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    void write(const T &t)
    {
        const std::lock_guard<std::mutex> lock(_m);
        _data = t;
        _fetched = false;
    }
};

TEST_F(DBFacadeTestFixture, MultithreadingBase)
{
    using namespace db;

    TableGuard<SomeMtRecord> someMtRecordTable(_storage);

    _storage.execute(query::insert<SomeMtRecord>({.id = 1, .attr1 = 1, .attr2 = 1}));
    _storage.execute(query::insert<SomeMtRecord>({.id = 2, .attr1 = 2, .attr2 = 2}));
    _storage.execute(query::insert<SomeMtRecord>({.id = 3, .attr1 = 3, .attr2 = 3}));

    auto t = std::thread([&]() {
        std::vector<SomeMtRecord> data = _storage.receive(
            query::select<SomeMtRecord>({&SomeMtRecord::id, &SomeMtRecord::attr1, &SomeMtRecord::attr2}));

        ASSERT_EQ(data.size(), 3);
        EXPECT_EQ(data.front().id, 1);
        EXPECT_EQ(data.back().id, 3);
    });
    t.join();
}

TEST_F(DBFacadeTestFixture, MultithreadingParallelOneConnectionRW)
{
    using namespace db;

    Connection::SPtr connection(new db::SqliteConnection(":memory:"));
    Facade storage(connection);
    LockedVar<SomeMtRecord> expectedRecord;
    bool stopReading = false;

    EXPECT_NO_THROW(storage.execute(query::createTable<SomeMtRecord>()));

    auto inserter = std::thread([&storage, &expectedRecord, &stopReading]() {
        for (int i = 0; i < 100; ++i)
        {
            SomeMtRecord record{.id = i, .attr1 = i, .attr2 = i};
            EXPECT_NO_THROW(storage.execute(query::insert(record)));
            expectedRecord.write(record);
        }
        stopReading = true;
    });

    auto reader = std::thread([&storage, &expectedRecord, &stopReading]() {
        SomeMtRecord expected;
        while (!stopReading)
        {
            if (expectedRecord.fetch(expected))
            {
                std::vector<SomeMtRecord> data;

                EXPECT_NO_THROW(
                    data = storage.receive(
                        query::select<SomeMtRecord>({&SomeMtRecord::id, &SomeMtRecord::attr1, &SomeMtRecord::attr2})
                            .where(db::field(&SomeMtRecord::id) == expected.id)));
                ASSERT_EQ(data.size(), 1);
                EXPECT_TRUE(isConsistent(data[0]));
            }
        }
    });
    inserter.join();
    reader.join();
}

TEST_F(DBFacadeTestFixture, MultithreadingConsequentTwoConnectionsRW)
{
    using namespace db;
    const char *db_name = "test_db_double_conn_rw";
    bool stopReading = false;

    Connection::SPtr readerconnection(new db::SqliteConnection(db_name));
    Facade readerstorage(readerconnection);
    Connection::SPtr writerconnection(new db::SqliteConnection(db_name));
    Facade writerstorage(writerconnection);

    writerstorage.execute(db::query::drop<SomeMtRecord>());

    EXPECT_NO_THROW(writerstorage.execute(query::createTable<SomeMtRecord>()));

    auto inserter = std::thread([&writerstorage, &stopReading]() {
        for (int i = 0; i < 100; ++i)
        {
            SomeMtRecord record{.id = i, .attr1 = i, .attr2 = i};
            writerstorage.execute(query::insert(record));
        }
        stopReading = true;
    });

    inserter.join();

    std::vector<SomeMtRecord> data;
    for (int i = 0; i < 100; ++i)
    {
        SomeMtRecord expected{.id = i, .attr1 = i, .attr2 = i};
        EXPECT_NO_THROW(data = readerstorage.receive(
                            query::select<SomeMtRecord>({&SomeMtRecord::id, &SomeMtRecord::attr1, &SomeMtRecord::attr2})
                                .where(db::field(&SomeMtRecord::id) == expected.id)));
        ASSERT_EQ(data.size(), 1);
        EXPECT_EQ(data[0].id, i);
        EXPECT_TRUE(isConsistent(data[0]));
    }

    readerstorage.execute(db::query::drop<SomeMtRecord>());
}

TEST_F(DBFacadeTestFixture, MultithreadingDoubleConnectionsRead)
{
    using namespace db;
    const char *db_name = "test_db_double_conn_read";

    Connection::SPtr prepconnection(new db::SqliteConnection(db_name));
    Facade prepstorage(prepconnection);
    ASSERT_NO_THROW(prepstorage.execute(db::query::drop<SomeMtRecord>()));
    ASSERT_NO_THROW(prepstorage.execute(query::createTable<SomeMtRecord>()));

    EXPECT_NO_THROW(prepstorage.execute(query::insert<SomeMtRecord>({.id = 3, .attr1 = 3, .attr2 = 3})));

    auto reader1 = std::thread([db_name]() {
        Connection::SPtr readerconnection(new db::SqliteConnection(db_name));
        Facade readerstorage(readerconnection);
        for (int i = 0; i < 100; ++i)
        {
            std::vector<SomeMtRecord> data;

            EXPECT_NO_THROW(data = readerstorage.receive(query::select<SomeMtRecord>(
                                {&SomeMtRecord::id, &SomeMtRecord::attr1, &SomeMtRecord::attr2})));
            if (!data.empty())
            {
                EXPECT_TRUE(isConsistent(data.back()));
            }
        }
    });

    auto reader2 = std::thread([db_name]() {
        Connection::SPtr readerconnection(new db::SqliteConnection(db_name));
        Facade readerstorage(readerconnection);
        for (int i = 0; i < 100; ++i)
        {
            std::vector<SomeMtRecord> data;

            EXPECT_NO_THROW(data = readerstorage.receive(query::select<SomeMtRecord>(
                                {&SomeMtRecord::id, &SomeMtRecord::attr1, &SomeMtRecord::attr2})));
            if (!data.empty())
            {
                EXPECT_TRUE(isConsistent(data.back()));
            }
        }
    });
    reader1.join();
    reader2.join();

    prepstorage.execute(db::query::drop<SomeMtRecord>());
}

TEST_F(DBFacadeTestFixture, MultithreadingMultiConnectionsRW)
{
    // This test is going to fail as soon as Sqlite3 does not
    // support simultanous read and write in several connections

    using namespace db;
    const char *db_name = "test_db_multi_conn_rw";
    bool stopReading = false;

    Connection::SPtr prepconnection(new db::SqliteConnection(db_name));
    Facade prepstorage(prepconnection);
    prepstorage.execute(db::query::drop<SomeMtRecord>());
    EXPECT_NO_THROW(prepstorage.execute(query::createTable<SomeMtRecord>()));

    auto inserter = std::thread([&stopReading, db_name]() {
        Connection::SPtr writerconnection(new db::SqliteConnection(db_name));
        Facade writerstorage(writerconnection);

        for (int i = 0; i < 100; ++i)
        {
            SomeMtRecord record{.id = i, .attr1 = i, .attr2 = i};
            ASSERT_NO_THROW(writerstorage.execute(query::insert(record)));
        }
        stopReading = true;
    });

    auto reader = std::thread([&stopReading, db_name]() {
        Connection::SPtr readerconnection(new db::SqliteConnection(db_name));
        Facade readerstorage(readerconnection);
        while (!stopReading)
        {
            std::vector<SomeMtRecord> data;

            ASSERT_NO_THROW(data = readerstorage.receive(query::select<SomeMtRecord>(
                                {&SomeMtRecord::id, &SomeMtRecord::attr1, &SomeMtRecord::attr2})));
            if (!data.empty())
            {
                EXPECT_TRUE(isConsistent(data.back()));
            }
        }
    });
    inserter.join();
    stopReading = true; // To stop the reader in case if writer thread failed on assertion.
    reader.join();
    prepstorage.execute(db::query::drop<SomeMtRecord>());
}

