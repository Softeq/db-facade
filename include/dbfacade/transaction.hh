#ifndef SOFTEQ_DBFACADE_TRANSACTION_H_
#define SOFTEQ_DBFACADE_TRANSACTION_H_

#include "sqlquery.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class which represents a query which begins a transaction.
*/
class BeginTransactionQuery : public SerializableSqlQuery<BeginTransactionQuery>
{
public:
    explicit BeginTransactionQuery();
};

/*!
    \brief Class which represents a query which commits a transaction.
*/
class CommitTransactionQuery : public SerializableSqlQuery<CommitTransactionQuery>
{
public:
    explicit CommitTransactionQuery();
};

/*!
    \brief Class which represents a query which rolls back a transaction.
*/
class RollbackTransactionQuery : public SerializableSqlQuery<RollbackTransactionQuery>
{
public:
    explicit RollbackTransactionQuery();
};

namespace query
{
/*!
    \brief Method forms a BEGIN TRANSACTION query.
    Do not use this method directly unless you have to. Prefer
    using db::Facade::execTransaction method
*/
inline BeginTransactionQuery beginTransaction()
{
    return BeginTransactionQuery();
}

/*!
    \brief Method forms a CIMMIT query.
    Do not use this method directly unless you have to. Prefer
    using db::Facade::execTransaction method
*/
inline CommitTransactionQuery commitTransaction()
{
    return CommitTransactionQuery();
}
/*!
    \brief Method forms a ROLLBACK query.
    Do not use this method directly unless you have to. Prefer
    using db::Facade::execTransaction method
*/
inline RollbackTransactionQuery rollbackTransaction()
{
    return RollbackTransactionQuery();
}
} // namespace query

} // namespace db
} // namespace softeq

#endif
