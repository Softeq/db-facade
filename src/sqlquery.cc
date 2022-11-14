#include "sqlquery.hh"

namespace softeq
{
namespace db
{
SqlQuery::SqlQuery(const TableScheme &scheme)
    : _scheme(scheme)
{
}

void SqlQuery::setCells(std::vector<Cell> &&cells)
{
    _cells = std::move(cells);
}

const std::vector<Cell> &SqlQuery::cells() const
{
    return _cells;
}

const std::string &SqlQuery::table() const
{
    return _scheme.name();
}

const TableScheme &SqlQuery::scheme() const
{
    return _scheme;
}

} // namespace db
} // namespace softeq
