#include "sqlexception.hh"
#include <sqlite3.h>

namespace softeq
{
namespace db
{

SqlException::SqlException(const std::string &message)
: std::runtime_error(message)
{
}

} // namespace db
} // namespace softeq
