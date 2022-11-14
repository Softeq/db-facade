#include "createtable.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
CreateTableQuery::CreateTableQuery(const TableScheme &scheme)
    : SqlConditionalQuery(scheme)
{
    setCells(scheme.cells());
}

CreateTableQuery &CreateTableQuery::where(const Condition &condition)
{
    SqlConditionalQuery::where(condition);
    return *this;
}

CreateTableQuery &CreateTableQuery::orderBy(const OrderBy &order)
{
    _orderbys.push_back(order);
    return *this;
}

const std::vector<OrderBy> &CreateTableQuery::orderBys() const
{
    return _orderbys;
}

const TableScheme &CreateTableQuery::schemeSource() const
{
    return _schemeSource;
}

} // namespace db
} // namespace softeq
