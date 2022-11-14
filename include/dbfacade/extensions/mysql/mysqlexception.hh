#ifndef SOFTEQ_MYSQL_MYSQLEXCEPTION_H_
#define SOFTEQ_MYSQL_MYSQLEXCEPTION_H_

#include <dbfacade/sqlexception.hh>

namespace softeq
{
namespace db
{
namespace mysql
{
class MySqlException : public SqlException
{
public:
    explicit MySqlException(const std::string &message);
    explicit MySqlException(const std::string &message, const std::string &query);
};

} // namespace mysql
} // namespace db
} // namespace softeq

#endif // SOFTEQ_MYSQL_MYSQLEXCEPTION_H_
