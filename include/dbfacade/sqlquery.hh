#ifndef SOFTEQ_DBFACADE_SQLQUERY_H_
#define SOFTEQ_DBFACADE_SQLQUERY_H_

#include "tablescheme.hh"
#include "condition.hh"
#include "sqlquerybuilder.hh"
#include "base_constraint.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Basic interface for implementing queries.
    You must inherit from it if you want to implement your own request type.
*/
class SqlQuery
{
public:
    SqlQuery() = delete;
    explicit SqlQuery(const TableScheme &scheme);
    virtual ~SqlQuery() = default;

    virtual std::vector<Statement> buildStatement(const SqlQueryStringBuilder &) const = 0;

    /*!
        \brief Method puts in the query a collection of table cells that will participate in the query to the database
        \param[in] cells A prepared collection with cells for creating a query to the database
    */
    void setCells(std::vector<Cell> &&cells);

    /*!
        \brief The method returns a reference to the collection of table cells that are stored in the query.
        \return Collection with cells for database query
    */
    const std::vector<Cell> &cells() const; // TODO: consider returning by value

    /*!
        \brief Method returns the name of the table that belongs to this query
        \return string with the table name from this query
    */
    const std::string &table() const; // TODO: consider returning by value

    const TableScheme &scheme() const; // TODO: consider returning by value

private:
    TableScheme _scheme;
    std::vector<Cell> _cells;
};

/*!
    \brief The class implements toString method to avoid code duplication.
    \tparam SqlQueryImplType a class that implements a specific SQL construction
    (e.g. select, insert, update etc.)
*/
template <typename SqlQueryImplType>
class SerializableSqlQuery : public SqlQuery
{
public:
    template <typename... Args>
    SerializableSqlQuery(Args &&... args)
        : SqlQuery(std::forward<Args>(args)...)
    {
    }

    std::vector<Statement> buildStatement(const SqlQueryStringBuilder &builder) const override
    {
        return builder.buildStatement(static_cast<const SqlQueryImplType &>(*this));
    }
};

/*!
    \brief The class implements condition functionality for queries that require it.
    \tparam SqlQueryImplType a class that implements a specific SQL construction
    (e.g. select, insert, update etc.)
*/
template <typename SqlQueryImplType>
class SqlConditionalQuery : public SerializableSqlQuery<SqlQueryImplType>
{
public:
    explicit SqlConditionalQuery(const TableScheme &scheme)
        : SerializableSqlQuery<SqlQueryImplType>(scheme)
    {
    }

    /*!
        \brief The method accepts and stores in the object an additional condition in the object to the query in the
        database. The condition will be used when accessing the database directly inside the perform method
        \param[in] condition Condition class object with database query condition
        \return this SqlQuery object with the added query condition
    */
    virtual SqlQuery &where(const Condition &condition)
    {
        _condition = std::move(condition);
        return *this;
    }

    /*!
        \brief Method returns an additional condition in the object to the query in the database
        \return Condition class object with database query condition
    */
    virtual const Condition &condition() const
    {
        return _condition;
    }

private:
    Condition _condition;
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SQLQUERY_H_
