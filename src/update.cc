#include "update.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
UpdateQuery::UpdateQuery(const TableScheme &scheme)
    : SqlConditionalQuery(scheme)
{
}

} // namespace db
} // namespace softeq
