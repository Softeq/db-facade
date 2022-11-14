#include "transaction.hh"

namespace softeq
{
namespace db
{
BeginTransactionQuery::BeginTransactionQuery()
    : SerializableSqlQuery(TableScheme{})
{
}

CommitTransactionQuery::CommitTransactionQuery()
    : SerializableSqlQuery(TableScheme{})
{
}

RollbackTransactionQuery::RollbackTransactionQuery()
    : SerializableSqlQuery(TableScheme{})
{
}

} // namespace db
} // namespace softeq
