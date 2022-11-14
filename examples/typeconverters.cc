#include <dbfacade/sqliteconnection.hh>
#include <dbfacade/columndatetime.hh>
#include <dbfacade/createtable.hh>
#include <dbfacade/facade.hh>
#include <dbfacade/insert.hh>
#include <dbfacade/select.hh>
#include <dbfacade/drop.hh>
#include <dbfacade/columntypes.hh>
#include <dbfacade/columndatetime.hh>
#include <cstring>
#include <iostream>

using namespace softeq;

namespace IP
{
struct IP
{
    int a, b, c, d;
};

std::string serialize(const IP &ip)
{
    std::stringstream ss;
    ss << "'" << ip.a << '.' << ip.b << '.' << ip.c << '.' << ip.d << "'";
    return ss.str();
}

IP deserialize(const std::string &ipstr)
{
    IP ip;
    char c;
    std::stringstream ss(ipstr);
    ss >> ip.a >> c >> ip.b >> c >> ip.c >> c >> ip.d;
    return ip;
}

bool operator==(const IP &one, const IP &another)
{
    return memcmp(&one, &another, sizeof(IP)) == 0;
}
/**
 * \brief Helper function for creation unique_ptr for IP structure
 */
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

} // end namespace IP

/**
 * \brief Place a custom serializer under softeq::db::type_serializers namespace
 */
namespace softeq::db::type_serializers
{
/**
 * \brief To add serializer just specialize 'serialize' structure with following static functions:
 * getTypeHint() used to make a hint to Database type deduction algorithm
 * \see from fo serialize from the following type to DB serialized type
 * \see to to reconstruct type from the database type
 */
template <>
struct serialize<IP::IP>
{
    /**
     * \brief Add a hint to the dbfacade to select most suitable type
     */
    static TypeHint getTypeHint()
    {
        return TypeHint(TypeHint::InnerType::Binary);
    }

    /**
     * \brief Serializer from target type to the dbfacade internal type
     */
    static SqlValue from(const IP::IP &from)
    {
        std::stringstream ss;
        ss << from.a << '.' << from.b << '.' << from.c << '.' << from.d;

        return SqlValue(ss.str());
    }

    /**
     * \brief Deserializer from db internal type to the target type
     */
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

struct Student
{
    int id;
    std::string name;
    IP::IP ip;
    std::unique_ptr<IP::IP> ip2;
    std::time_t time;
    std::vector<int> marks;
};

bool operator==(const Student &one, const Student &another)
{
    return one.id == another.id && one.name == another.name && one.ip == another.ip && one.ip2 == another.ip2;
}

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
                    return db::SqlValue::Null();
                }

                std::stringstream ss;
                std::copy(dataToSerialize.begin(), std::prev(dataToSerialize.end()), std::ostream_iterator<T>(ss, ","));
                ss << *std::prev(dataToSerialize.end());

                return db::SqlValue(ss.str());
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
            {&Student::ip, "ip"},  // Not Nullable type
            {&Student::ip2, "ip2", db::columntypes::Nullable}, // make this field nullable
            {&Student::time, "time", db::columntypes::DateTime},
            {&Student::marks, "marks", commaSeparatedConverter},
        }
    ); // clang-format on
    return scheme;
}

int main()
{
    using namespace softeq;
    using namespace db;

    Connection::SPtr connection(new SqliteConnection(":memory:"));
    Facade storage(connection);

    storage.execute(query::createTable<Student>());

    //-----------------------------------------------------------------------------------

    std::vector<Student> insertData;

    Student s1{.id = 1,
               .name = "John",
               .ip = {192, 168, 0, 1},
               .ip2 = IP::makeUIP(192, 168, 0, 1),
               .time = 1645195523,
               .marks = {9, 9, 8}};
    Student s2{.id = 2,
               .name = "Jane",
               .ip = {192, 168, 0, 2},
               .ip2 = std::unique_ptr<IP::IP>(),
               .time = 1645195524,
               .marks = {7, 8, 9}};

    insertData.emplace_back(std::move(s1));
    insertData.emplace_back(std::move(s2));

    for (auto &row : insertData)
    {
        storage.execute(query::insert<Student>(row));
    }

    std::vector<Student> receivedData = storage.receive(query::select<Student>({}));

    if (insertData == receivedData)
    {
        std::cout << "Serialize is OK" << std::endl;
    }

    //-----------------------------------------------------------------------------------

    // Drop database

    // DROP TABLE IF EXISTS SomeTable;
    storage.execute(db::query::drop<Student>());
}
