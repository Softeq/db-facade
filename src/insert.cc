#include "insert.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
InsertQuery::InsertQuery(const TableScheme &scheme)
    : SerializableSqlQuery(scheme)
{
}

} // namespace db
} // namespace softeq
