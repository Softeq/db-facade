#include "facade.hh"
#include "transaction.hh"

namespace softeq
{
namespace db
{
void Facade::execute(const SqlQuery &query) const
{
    _connection->perform(query);
}

void Facade::beginTransaction()
{
    execute(query::beginTransaction());
}

void Facade::endTransaction(bool commit)
{
    if (commit)
    {
        execute(query::commitTransaction());
    }
    else
    {
        execute(query::rollbackTransaction());
    }
}

void Facade::execTransaction(std::function<bool(Facade &)> transactionFunction)
{
    beginTransaction();
    bool commit = transactionFunction(*this);
    endTransaction(commit);
}

} // namespace db
} // namespace softeq
