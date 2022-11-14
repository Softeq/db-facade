#ifndef SOFTEQ_DBFACADE_CREATETABLE_H_
#define SOFTEQ_DBFACADE_CREATETABLE_H_

#include "sqlquery.hh"
#include "orderby.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class for create a new table in the database
*/
class CreateTableQuery : public SqlConditionalQuery<CreateTableQuery>
{
public:
    explicit CreateTableQuery(const TableScheme &scheme);

    /*!
        \brief The method accepts and stores in the object an additional condition in the object to the query in the
        database. The condition will be used when accessing the database directly inside the perform method
        \param[in] condition Condition class object with database query condition
        \return this CreateTableQuery object with the added query condition
    */
    CreateTableQuery &where(const Condition &condition) override;

    /*!
        \brief The method accepts and stores an ORDER BY clause in the query object
        It will be used when accessing the database directly inside the perform method.
        \param[in] order OrderBy object with columns and order (ASC, DESC)
        \return this TypedQuery object with added ORDER BY clause
    */
    CreateTableQuery &orderBy(const OrderBy &order);

    /*!
        \brief Method returns the vector of ORDER BY clauses for a query.
        \return a vector of ORDER BY objects
    */
    const std::vector<OrderBy> &orderBys() const;

    /*!
        \brief Method returns the table TableScheme class.
        \return a TableScheme objects
    */
    const TableScheme &schemeSource() const;

    /*!
        \brief The method specifies subquery which should be used to get data from
        another table while creating new onw.
        \return TypedQuery object
    */
    template <typename AnotherSctruct>
    CreateTableQuery &asSelect()
    {
        _schemeSource = buildTableScheme<AnotherSctruct>();
        return *this;
    }

private:
    TableScheme _schemeSource;
    std::vector<OrderBy> _orderbys;
};

namespace query
{
/*!
    \brief Forms a query for CREATE new empty table in the database that matches the specified pattern
    \tparam <Struct> containing the schema of the database table
*/
template <typename Struct>
CreateTableQuery createTable()
{
    return CreateTableQuery(buildTableScheme<Struct>());
}

} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_CREATETABLE_H_
