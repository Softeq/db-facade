#include "mysqlexception.hh"

namespace softeq
{
namespace db
{
namespace mysql
{

MySqlException::MySqlException(const std::string &message)
: SqlException(message)
{
}

MySqlException::MySqlException(const std::string &message, const std::string &query)
: SqlException(message + ": " + query)
{
}

} // namespace mysql
} // namespace db
} // namespace softeq
