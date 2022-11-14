#include "testfixture.hh"

using namespace softeq;

namespace
{
struct Student
{
    int id;
    std::string name;
};

struct StudentMoreColumns : public Student
{
    std::time_t time;
};

struct StudentMoreNullColumns : public Student // same as StudentMoreColumns but with nullable 'time'
{
    std::unique_ptr<std::time_t> time;
};

struct StudentWithDifferentType // relative to StudentMoreColumns
{
    int id;
    std::string name;
    std::string time;
};

struct StudentWithOtherFlags : public Student
{
    // same members
};

struct StudentWithOtherDefault : public Student
{
    // same members
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

template <>
const db::TableScheme db::buildTableScheme<StudentMoreColumns>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&StudentMoreColumns::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&StudentMoreColumns::name, "name"},
            {&StudentMoreColumns::time, "time"} // additional column
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<StudentMoreNullColumns>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&StudentMoreNullColumns::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&StudentMoreNullColumns::name, "name"},
            {&StudentMoreNullColumns::time, "time", db::columntypes::Nullable} // additional column, nullable
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<StudentWithDifferentType>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&StudentWithDifferentType::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&StudentWithDifferentType::name, "name"},
            {&StudentWithDifferentType::time, "time"} // different type (INTEGER)
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<StudentWithOtherFlags>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&StudentWithOtherFlags::id, "id"}, // no PK
            {&StudentWithOtherFlags::name, "name"}
        }
    ); // clang-format on
    return scheme;
}

template <>
const db::TableScheme db::buildTableScheme<StudentWithOtherDefault>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&StudentWithOtherDefault::id, "id", db::Cell::Flags::PRIMARY_KEY}, 
            {&StudentWithOtherDefault::name, "name", db::Cell::Flags::DEFAULT, "John Doe"}
        }
    ); // clang-format on
    return scheme;
}

#define EXPECT_THROW_WITH_TEXT(operation, exceptionType, expectedText)                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        try                                                                                                            \
        {                                                                                                              \
            operation;                                                                                                 \
            FAIL() << "Expected to throw an exception ";                                                               \
        }                                                                                                              \
        catch (exceptionType & ex)                                                                                     \
        {                                                                                                              \
            EXPECT_STREQ(ex.what(), expectedText);                                                                     \
        }                                                                                                              \
    } while (0)

TEST_F(DBFacadeTestFixture, VerifyGood)
{
    using namespace db;

    // this is supposed to be what we have, i.e. a table we created sometime previousely
    TableGuard<Student> dbguard(_storage);

    // this is what we expect it to be
    EXPECT_NO_THROW(_storage.verifyScheme<Student>());
}

TEST_F(DBFacadeTestFixture, VerifyMoreColumns) // more columns than we need
{
    using namespace db;

    TableGuard<StudentMoreColumns> dbguard(_storage);
    EXPECT_THROW_WITH_TEXT(_storage.verifyScheme<Student>(), SqlException,
                           "Column 'time' does not exist in the scheme");
}

TEST_F(DBFacadeTestFixture, VerifyLessColumns) // less columns than we need
{
    using namespace db;

    TableGuard<Student> dbguard(_storage);
    EXPECT_THROW_WITH_TEXT(_storage.verifyScheme<StudentMoreColumns>(), SqlException,
                           "Column 'time' from scheme does not exist in the table");
}

TEST_F(DBFacadeTestFixture, VerifyDifferentTypes) // three columns, but different types
{
    using namespace db;

    TableGuard<StudentWithDifferentType> dbguard(_storage);
    // The message should be like this:
    // "Type INTEGER of column 'time' does not match type DATETIME in scheme"
    // but since different DBMS may have different type names, it's hard to verify the message.
    EXPECT_THROW(_storage.verifyScheme<StudentMoreColumns>(), SqlException);
}

TEST_F(DBFacadeTestFixture, VerifyDifferentFlags) // same columns, but different flags
{
    using namespace db;

    {
        TableGuard<Student> dbguard(_storage);
        EXPECT_THROW_WITH_TEXT(_storage.verifyScheme<StudentWithOtherFlags>(), SqlException,
                               "Parameters (4) of column 'id' do not match expected parameters (0)");
    }

    {
        TableGuard<StudentMoreColumns> dbguard(_storage);
        EXPECT_THROW_WITH_TEXT(_storage.verifyScheme<StudentMoreNullColumns>(), SqlException,
                               "The value of the nullable flag for column 'time' has value (0) and does not match expected (1)");
    }

    {
        TableGuard<StudentMoreNullColumns> dbguard(_storage);
        EXPECT_THROW_WITH_TEXT(_storage.verifyScheme<StudentMoreColumns>(), SqlException,
                               "The value of the nullable flag for column 'time' has value (1) and does not match expected (0)");
    }
}

TEST_F(DBFacadeTestFixture, VerifyDifferentDefault) // same columns, but different default value
{
    using namespace db;

    TableGuard<Student> dbguard(_storage);
    EXPECT_THROW_WITH_TEXT(_storage.verifyScheme<StudentWithOtherDefault>(), SqlException,
                           "Default value '' of column 'name' does not match expected value John Doe");
}
