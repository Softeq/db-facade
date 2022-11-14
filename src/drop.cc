#include "drop.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
DropQuery::DropQuery(const TableScheme &scheme)
    : SerializableSqlQuery(scheme)
{
}

} // namespace db
} // namespace softeq
