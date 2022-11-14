#include "testfixture.hh"
#include <dbfacade/remove.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>
#include <dbfacade/update.hh>
#include <dbfacade/constraints.hh>

using namespace softeq;
using namespace softeq::db::constraints;

// Cascade

namespace
{
struct SomeCascadeParent
{
    int id;
    int sec_id;
};

struct SomeCascadeChild
{
    int id;
    int ref_id;
};
} // namespace

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

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeCascadeChild>()
{
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

TEST_F(DBFacadeTestFixture, CascadeBase)
{
    using namespace db;

    TableGuard<SomeCascadeParent> someCascadeParentTable(_storage);
    TableGuard<SomeCascadeChild> someCascadeChildTable(_storage);

    _storage.execute(query::insert<SomeCascadeParent>({.id = 1, .sec_id = 10}));
    _storage.execute(query::insert<SomeCascadeParent>({.id = 2, .sec_id = 20}));
    _storage.execute(query::insert<SomeCascadeParent>({.id = 3, .sec_id = 30}));

    EXPECT_NO_THROW(_storage.execute(query::insert<SomeCascadeChild>({.id = 2, .ref_id = 2})));

    std::vector<SomeCascadeChild> data =
        _storage.receive(query::select<SomeCascadeChild>({&SomeCascadeChild::id, &SomeCascadeChild::ref_id}));
    EXPECT_EQ(data.size(), 1);

    EXPECT_NO_THROW(_storage.execute(query::remove<SomeCascadeParent>().where(db::field(&SomeCascadeParent::id) == 2)));

    data = _storage.receive(query::select<SomeCascadeChild>({&SomeCascadeChild::id, &SomeCascadeChild::ref_id}));
    EXPECT_EQ(data.size(), 0);
}

// Restrict + SetNull

namespace
{
struct RestrictSetNullParent
{
    int id;
    int sec_id;
};

struct RestrictSetNullChild
{
    int id;
    std::unique_ptr<int> ref_id;
};
} // namespace

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<RestrictSetNullParent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("ParentTable", 
        {
            {&RestrictSetNullParent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&RestrictSetNullParent::sec_id, "sec_id"}
        }
    ); // clang-format on
    return scheme;
}

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<RestrictSetNullChild>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("ChildTable", 
        {
            {&RestrictSetNullChild::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&RestrictSetNullChild::ref_id, "ref_id", db::columntypes::Nullable}
        },
        {
            makeConstraint<ForeignKeyConstraint>(&RestrictSetNullChild::ref_id, &RestrictSetNullParent::id, 
                            addCascade(CascadeTrigger::OnUpdate, CascadeAction::Restrict),
                            addCascade(CascadeTrigger::OnDelete, CascadeAction::SetNull))
        //     // "r_ChildTable",
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, RestrictSetNull)
{
    using namespace db;

    // prepare data

    TableGuard<RestrictSetNullParent> someCascadeParentTable(_storage);
    TableGuard<RestrictSetNullChild> someCascadeChildTable(_storage);

    _storage.execute(query::insert<RestrictSetNullParent>({.id = 1, .sec_id = 10}));
    _storage.execute(query::insert<RestrictSetNullParent>({.id = 2, .sec_id = 20}));
    _storage.execute(query::insert<RestrictSetNullParent>({.id = 3, .sec_id = 30}));

    EXPECT_NO_THROW(_storage.execute(query::insert<RestrictSetNullChild>({.id = 2, .ref_id = std::unique_ptr<int>(new int(2))})));

    // do tests

    RestrictSetNullParent updatedData{};
    updatedData.id = 4;
    // should restrict
    EXPECT_THROW(
        _storage.execute(
            query::update({&RestrictSetNullParent::id}, updatedData).where(field(&RestrictSetNullParent::id) == 2)),
        SqlException);

    // should set to null
    EXPECT_NO_THROW(
        _storage.execute(query::remove<RestrictSetNullParent>().where(db::field(&RestrictSetNullParent::id) == 2)));

    std::vector<RestrictSetNullChild> data = _storage.receive(
        query::select<RestrictSetNullChild>({&RestrictSetNullChild::id, &RestrictSetNullChild::ref_id}));
    EXPECT_TRUE(data.at(0).ref_id.get() == nullptr);
}

// Loop constraints

struct SomeLoopedParent
{
    int id;
    int ref_id;
};
struct SomeLoopedChild
{
    int id;
    int ref_id;
};

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeLoopedParent>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("ParentLoopedTable", 
        {
            {&SomeLoopedParent::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeLoopedParent::ref_id, "ref_id"}
        },
        {
            makeConstraint<ForeignKeyConstraint>(&SomeLoopedParent::ref_id, &SomeLoopedChild::id)
        }
    ); // clang-format on
    return scheme;
};

template <>
const softeq::db::TableScheme softeq::db::buildTableScheme<SomeLoopedChild>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("ChildLoopedTable", 
        {
            {&SomeCascadeChild::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&SomeCascadeChild::ref_id, "ref_id"}
        },
        {
            makeConstraint<ForeignKeyConstraint>(&SomeLoopedChild::ref_id, &SomeLoopedParent::id)
        }
    ); // clang-format on
    return scheme;
};

TEST_F(DBFacadeTestFixture, DISABLED_CascadeTablesLooped)
{
    using namespace db;

    // TODO: This case leads to an infinite loop, you need to figure out how to avoid this

    EXPECT_NO_THROW(_storage.execute(db::query::createTable<SomeLoopedParent>()));
    EXPECT_NO_THROW(_storage.execute(db::query::createTable<SomeLoopedChild>()));

    EXPECT_NO_THROW(_storage.execute(db::query::drop<SomeLoopedParent>()));
    EXPECT_NO_THROW(_storage.execute(db::query::drop<SomeLoopedChild>()));
}
