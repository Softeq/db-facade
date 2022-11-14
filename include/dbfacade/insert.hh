#ifndef SOFTEQ_DBFACADE_INSERT_H_
#define SOFTEQ_DBFACADE_INSERT_H_

#include "sqlquery.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class for adding new data to the table
*/
class InsertQuery : public SerializableSqlQuery<InsertQuery>
{
public:
    explicit InsertQuery(const TableScheme &scheme);
};

namespace query
{
/*!
    \brief Forms an INSERT query that puts data into the database
    \tparam <Struct> containing the schema of the database table if the type is passed implicitly
    \param[in] data A Struct filled with data to be inserted into a table
*/
template <typename Struct>
InsertQuery insert(const Struct &data)
{
    auto scheme = buildTableScheme<Struct>();
    std::vector<Cell> cells = scheme.cells();
    for (Cell &cell : cells)
    {
        cell.serialize(data);
    }

    InsertQuery query(scheme);
    query.setCells(std::move(cells));

    return query;
}

/*!
    \brief Forms an INSERT query that puts data into the database
    \tparam <Struct> containing the schema of the database table if the type is passed implicitly
    \param[in] fields fields to insert; the rest will be filled automatically
        using default values or calculated fields
    \param[in] data A Struct filled with data to be inserted into a table
*/
template <typename Struct>
InsertQuery insert(std::initializer_list<CellMaker> fields, const Struct &data)
{
    std::vector<Cell> cells;
    for (const CellMaker &field : fields)
    {
        cells.emplace_back(field());
        cells.back().serialize(data);
    }

    InsertQuery query(buildTableScheme<Struct>());
    query.setCells(std::move(cells));

    return query;
}

} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_INSERT_H_
