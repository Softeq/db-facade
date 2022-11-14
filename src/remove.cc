#include "remove.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
RemoveQuery::RemoveQuery(const TableScheme &scheme)
    : SqlConditionalQuery(scheme)
{
}

} // namespace db
} // namespace softeq
