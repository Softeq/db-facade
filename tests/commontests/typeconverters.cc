#include "testfixture.hh"

#include <dbfacade/sqlvalue.hh>
#include <dbfacade/columndatetime.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>
#include <dbfacade/typeserializers.hh>
#include <dbfacade/typehint.hh>

#include <chrono>
#include <memory>

using namespace softeq;

namespace
{
namespace IP
{
struct IP
{
    int a, b, c, d;
};

bool operator==(const IP &one, const IP &another)
{
    return memcmp(&one, &another, sizeof(IP)) == 0;
}

std::unique_ptr<IP> makeUIP(int a, int b, int c, int d)
{
    return std::unique_ptr<IP>(new IP{a, b, c, d});
}

/**
 * \brief Helper function for comparing unique pointers to IP structures
 */
bool operator==(const std::unique_ptr<IP> &lhs, const std::unique_ptr<IP> &rhs)
{
    if (!lhs && !rhs)
    {
        return true;
    }
    if (lhs && rhs)
    {
        return *lhs == *rhs;
    }
    return false;
}

} // namespace IP

struct Student
{
    int id;
    std::string name;
    IP::IP ip;
    std::unique_ptr<IP::IP> ip2;
    std::time_t time;
    std::vector<int> marks;
    std::int64_t data64;
    std::uint16_t data16;
};

bool operator==(const Student &one, const Student &another)
{
    return one.id == another.id && one.name == another.name && one.ip == another.ip && one.ip2 == another.ip2;
}
} // namespace

namespace softeq::db::type_serializers
{
template <>
struct serialize<IP::IP>
{
    static TypeHint getTypeHint()
    {
        return TypeHint(TypeHint::InnerType::Binary);
    }

    static SqlValue from(const IP::IP &from)
    {
        std::stringstream ss;
        ss << from.a << '.' << from.b << '.' << from.c << '.' << from.d;

        return SqlValue(ss.str());
    }

    static IP::IP to(const char *from)
    {
        IP::IP ip;
        char c;
        std::stringstream ss(from);
        ss >> ip.a >> c >> ip.b >> c >> ip.c >> c >> ip.d;
        return ip;
    }
};

} // namespace softeq::db::type_serializers

template <typename T>
void commaSeparatedConverter(db::TypeConverter<std::vector<T>> &converter)
{
    converter = {
        .isNullable = true,
        .typeHash = typeid(std::string).hash_code(),
        .from =
            [](const std::vector<T> &dataToSerialize) {
                if (dataToSerialize.empty())
                {
                    return softeq::db::SqlValue::Null();
                }

                std::stringstream ss;
                std::copy(dataToSerialize.begin(), std::prev(dataToSerialize.end()), std::ostream_iterator<T>(ss, ","));
                ss << *std::prev(dataToSerialize.end());

                return softeq::db::SqlValue(ss.str());
            },
        .to =
            [](const char *str) {
                std::vector<T> result;
                if (str == nullptr)
                {
                    return result;
                }

                std::stringstream ss(str);
                char delimiter;
                while (ss)
                {
                    T t{};
                    ss >> t >> delimiter;
                    result.push_back(t);
                }
                return result;
            }
        // (this comment is for clang-format)
    };
}

template <>
const db::TableScheme db::buildTableScheme<Student>()
{
    // clang-format off
    static const auto scheme = db::TableScheme("student",
        {
            {&Student::id, "id", db::Cell::Flags::PRIMARY_KEY},
            {&Student::name, "name"},
            {&Student::ip, "ip"},
            {&Student::ip2, "ip2", db::columntypes::Nullable},
            {&Student::time, "time", db::columntypes::DateTime},
            {&Student::marks, "marks", commaSeparatedConverter},
            {&Student::data64, "data64"},
            {&Student::data16, "data16"},
        }
    ); // clang-format on
    return scheme;
}

TEST_F(DBFacadeTestFixture, SerializersBasic)
{
    namespace sql = db::query;

    // create table
    TableGuard<Student> studentTable(_storage);

    Student s1{.id = 1,
               .name = "John",
               .ip = {192, 168, 0, 1},
               .ip2 = IP::makeUIP(192, 168, 0, 1),
               .time = 1645195523,
               .marks = {9, 9, 8},
               .data64 = -64,
               .data16 = 16};
    Student s2{.id = 2,
               .name = "Jane",
               .ip = {192, 168, 0, 2},
               .ip2 = IP::makeUIP(192, 168, 0, 2),
               .time = 1645195524,
               .marks = {7, 8, 9, 10},
               .data64 = -65,
               .data16 = 17};
    Student s3{.id = 3,
               .name = "Jack",
               .ip = {192, 168, 0, 3},
               .ip2 = std::unique_ptr<IP::IP>(),
               .time = 1645195525,
               .marks = {},
               .data64 = -66,
               .data16 = 18};

    std::vector<Student> sourceData;
    sourceData.emplace_back(std::move(s1));
    sourceData.emplace_back(std::move(s2));
    sourceData.emplace_back(std::move(s3));

    for (auto &row : sourceData)
    {
        _storage.execute(sql::insert<Student>(row));
    }

    // check the table exists
    std::vector<Student> receivedData = _storage.receive(sql::select<Student>({}));
    EXPECT_EQ(sourceData, receivedData);
}

TEST_F(DBFacadeTestFixture, SerializersEpochTime)
{
    const std::time_t sampleTime = 1648118445;
    const std::string sampleTimeStr = "2022-03-24 10:40:45.000";

    // temp variables
    std::time_t time;
    softeq::db::SqlValue timestr;

    softeq::db::TypeConverter<std::time_t> dateTime;
    softeq::db::columntypes::DateTime(dateTime);

    EXPECT_NO_THROW(timestr = dateTime.from(sampleTime));
    EXPECT_EQ(timestr.strValue(), sampleTimeStr);

    EXPECT_NO_THROW(time = dateTime.to(sampleTimeStr.c_str()));
    EXPECT_EQ(time, sampleTime);

    const std::string badSampleTimeStr = "2022/03/24 10:40:45.000"; // wrong format
    EXPECT_THROW(dateTime.to(badSampleTimeStr.c_str()), std::invalid_argument);
}
