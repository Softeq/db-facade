#include "select.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
SelectQuery::SelectQuery(const TableScheme &scheme)
    : SqlConditionalQuery(scheme)
{
}

SelectQuery &SelectQuery::where(const Condition &condition)
{
    SqlConditionalQuery::where(condition);
    return *this;
}

SelectQuery &SelectQuery::orderBy(const OrderBy &order)
{
    _orderbys.push_back(order);
    return *this;
}

SelectQuery &SelectQuery::limit(std::uint64_t value)
{
    _limit.rowsLimit = value;
    return *this;
}

SelectQuery &SelectQuery::offset(std::uint64_t value)
{
    _limit.rowsOffset = value;
    return *this;
}

const ResultLimit &SelectQuery::limits() const
{
    return _limit;
}

const std::vector<OrderBy> &SelectQuery::orderBys() const
{
    return _orderbys;
}

const std::vector<Join> &SelectQuery::joins() const
{
    return _joins;
}

} // namespace db
} // namespace softeq
