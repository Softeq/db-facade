#ifndef SOFTEQ_MYSQL_MYSQLQUERYBUILDER_H_
#define SOFTEQ_MYSQL_MYSQLQUERYBUILDER_H_

#include <dbfacade/sqlquerybuilder.hh>
#include <dbfacade/transaction.hh>
#include <dbfacade/alter.hh>
#include <dbfacade/cellrepresentation.hh>

namespace softeq
{
namespace db
{
namespace mysql
{
class MySqlCellRepresentation : public CellRepresentation
{
public:
    std::string typeToCastType(const std::string &typeName) const override;
    std::string description(const Cell &cell) const override;
};

class MySqlQueryStringBuilder : public SqlQueryStringBuilder
{
public:
    MySqlQueryStringBuilder(CellRepresentation &cellRepr);

    std::vector<Statement> buildStatement(const class AlterQuery &query) const override;
    std::vector<Statement> buildStatement(const class BeginTransactionQuery &query) const override;
};

} // namespace mysql
} // namespace db
} // namespace softeq

#endif // SOFTEQ_MYSQL_MYSQLQUERYBUILDER_H_
