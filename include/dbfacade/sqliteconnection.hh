#ifndef SOFTEQ_DBFACADE_SQLITECONNECTION_H_
#define SOFTEQ_DBFACADE_SQLITECONNECTION_H_

#include "connection.hh"

struct sqlite3;

namespace softeq
{
namespace db
{
class SqliteQueryStringBuilder : public SqlQueryStringBuilder
{
protected:
    std::string limit(const ResultLimit &query) const override;

public:
    explicit SqliteQueryStringBuilder(CellRepresentation &cellRepr);

    std::vector<Statement> buildStatement(const class AlterQuery &query) const override;
};

class SqliteConnection : public Connection
{
public:
    explicit SqliteConnection(const std::string &dbName);
    ~SqliteConnection() override;

    void verifyScheme(const TableScheme &) override;

private:
    void performImpl(const std::vector<Statement> &query, const parseFunc &) override;
    SqlQueryStringBuilder &queryBuilder() override;
    void enableForeignKeySupport();
    void enableWaitingOnBusy();

private:
    sqlite3 *_db = nullptr;
    CellRepresentation _cellRepr;
    SqliteQueryStringBuilder _builder{_cellRepr};
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SQLITECONNECTION_H_
