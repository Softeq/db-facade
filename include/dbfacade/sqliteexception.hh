#ifndef SOFTEQ_DBFACADE_SQLITEEXCEPTION_H_
#define SOFTEQ_DBFACADE_SQLITEEXCEPTION_H_

#include "sqlexception.hh"

namespace softeq
{
namespace db
{
class SqliteException : public db::SqlException
{
public:
    explicit SqliteException(int errorCode);
    SqliteException(const std::string &message, int errorCode);
    SqliteException(const std::string &message, const std::string &query, int errorCode);
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SQLITEEXCEPTION_H_
