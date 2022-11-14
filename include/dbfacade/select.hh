#ifndef SOFTEQ_DBFACADE_SELECT_H_
#define SOFTEQ_DBFACADE_SELECT_H_

#include "sqlquery.hh"
#include "orderby.hh"
#include "resultlimit.hh"
#include "join.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class for selecting data in a specific table
*/
class SelectQuery : public SqlConditionalQuery<SelectQuery>
{
public:
    explicit SelectQuery(const TableScheme &scheme);

    /*!
        \brief The method accepts and stores in the object an additional condition in the object to the query in the
        database. The condition will be used when accessing the database directly inside the perform method
        \param[in] condition Condition class object with database query condition
        \return this TypedQuery object with the added query condition
    */
    SelectQuery &where(const Condition &condition) override;

    /*!
        \brief The method accepts and stores an ORDER BY clause in the query object
        It will be used when accessing the database directly inside the perform method.
        \param[in] order OrderBy object with columns and order (ASC, DESC)
        \return this TypedQuery object with added ORDER BY clause
    */
    SelectQuery &orderBy(const OrderBy &order);

    /*!
        \brief Method returns the vector of ORDER BY clauses for a query.
        \return a vector of ORDER BY objects
    */
    const std::vector<OrderBy> &orderBys() const;

    /*!
        \return vector Join object
    */
    const std::vector<Join> &joins() const;

    /*!
        \brief The method accepts and stores in the object a join clause condition in the object to the query in the
        database. It will be used when accessing the database directly inside the perform method
        \param[in] condition Condition class object with join condition
        \return this SelectQuery object with the added join clause
    */
    template <typename JoinedStruct>
    SelectQuery &join(const Condition &condition)
    {
        _joins.emplace_back(buildTableScheme<JoinedStruct>().name(), std::move(condition));
        return *this;
    }

    /*!
        \brief The method accepts and stores in the object the maximum number of rows that should be fetched
        \param[in] value the max number of rows
        \return this SelectQuery object with the added join clause
    */
    SelectQuery &limit(std::uint64_t value);

    /*!
        \brief The method accepts and stores in the object the offset at which the select result should be fetched
        \param[in] value the offset (in rows)
        \return this SelectQuery object with the added join clause
    */
    SelectQuery &offset(std::uint64_t value);

    /*!
        \return ResultLimit object which contains the limits (starting offset and the max number of rows) of query
    */
    const ResultLimit &limits() const;

private:
    std::vector<Join> _joins;
    std::vector<OrderBy> _orderbys;
    ResultLimit _limit;
};

namespace query
{
/*!
    \brief Forms a SELECT query containing the given fields of the selected table
    Please note that only those directly requested are valid data in the fields, all other fields contain garbage
    \tparam <Struct> containing the schema of the database table
    \param[in] fields initializer_list containing the links of the fields to be included in the Query
*/
template <typename Struct>
SelectQuery select(std::initializer_list<CellMaker> fields)
{
    std::vector<Cell> cells;
    for (const CellMaker &field : fields)
    {
        cells.emplace_back(field());
    }

    SelectQuery query(buildTableScheme<Struct>());
    query.setCells(std::move(cells));

    return query;
}
} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SELECT_H_
