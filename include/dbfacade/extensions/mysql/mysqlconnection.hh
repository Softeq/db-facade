#ifndef SOFTEQ_MYSQL_MYSQLCONNECTION_H_
#define SOFTEQ_MYSQL_MYSQLCONNECTION_H_

#include <dbfacade/cellrepresentation.hh>
#include <dbfacade/connection.hh>
#include <dbfacade/tablescheme.hh>
#include "mysqlquerybuilder.hh"

namespace softeq
{
namespace db
{
namespace mysql
{
class MySqlConnection : public Connection
{
public:
    MySqlConnection(const std::string &host, const int port, const std::string &user_name, const std::string &password,
                    const std::string &database);
    ~MySqlConnection() override;

    void verifyScheme(const TableScheme &scheme) override;

private:
    MySqlQueryStringBuilder &queryBuilder() override;
    void performImpl(const std::vector<Statement> &statement, const parseFunc &) override;

    MySqlCellRepresentation _cellRepr;
    MySqlQueryStringBuilder _builder{_cellRepr};
    struct MYSQL *_session;
};

} // namespace mysql
} // namespace db
} // namespace softeq

#endif // SOFTEQ_MYSQL_MYSQLCONNECTION_H_
