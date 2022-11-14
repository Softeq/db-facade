#include "sqliteexception.hh"
#include <sqlite3.h>

namespace softeq
{
namespace db
{
namespace
{

/**
 * \brief Helper function used to create an error message for the SqliteException
 *        This function gets an original error text and appends textual description
 *        for the provided sqlite error code
 * 
 * \param message    Error message 
 * \param errorCode  sqlite exception code
 * \return std::string 
 */
std::string makeErrorMessage(const std::string &message, int errorCode)
{
    std::string what(message);
    if (what.size() > 0)
    {
        what.append(": ");
    }
    what.append(sqlite3_errstr(errorCode));
    return what;
}

}

SqliteException::SqliteException(const std::string &message, int errorCode)
: db::SqlException(makeErrorMessage(message, errorCode))
{
}

SqliteException::SqliteException(const std::string &message, const std::string &query, int errorCode)
: db::SqlException(makeErrorMessage(message + " in '" + query + "'", errorCode))
{
}

SqliteException::SqliteException(int errorCode)
    : SqliteException(std::string(), errorCode)
{
}

} // namespace db
} // namespace softeq
