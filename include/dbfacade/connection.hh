#ifndef SOFTEQ_DBFACADE_CONNECTION_H_
#define SOFTEQ_DBFACADE_CONNECTION_H_

#include <memory>
#include <map>
#include <vector>
#include <string>

#include "sqlquery.hh"
#include "sqlquerybuilder.hh"
#include "sqlexception.hh"

namespace softeq
{
namespace db
{
class Connection
{
private:
    Connection(Connection &&) = delete;
    Connection(Connection const &) = delete;
    Connection &operator=(Connection &&) = delete;
    Connection &operator=(Connection const &) = delete;

public:
    Connection() = default;
    virtual ~Connection() = default;

    using SPtr = std::shared_ptr<Connection>;
    using WPtr = std::weak_ptr<Connection>;

    using parseFunc =
        std::function<void(const std::map<std::string, int> &header, const std::vector<const char *> &row)>;

    void perform(const SqlQuery &query, const parseFunc &pf = nullptr)
    {
        performImpl(query.buildStatement(queryBuilder()), pf);
    }

    virtual void verifyScheme(const TableScheme &) = 0;

protected:
    virtual void performImpl(const std::vector<Statement> &statements, const parseFunc &fn) = 0;

    virtual SqlQueryStringBuilder &queryBuilder() = 0;
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_CONNECTION_H_
