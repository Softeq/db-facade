#ifndef SOFTEQ_DBFACADE_SQLEXCEPTION_H_
#define SOFTEQ_DBFACADE_SQLEXCEPTION_H_

#include <stdexcept>
#include <exception>
#include <string>

namespace softeq
{
namespace db
{

//TODO: rename to Exception. Will be softeq::db:Exception
class SqlException : public std::runtime_error
{
public:
    explicit SqlException(const std::string &message);
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SQLEXCEPTION_H_
