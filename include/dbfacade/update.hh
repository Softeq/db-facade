#ifndef SOFTEQ_DBFACADE_UPDATE_H_
#define SOFTEQ_DBFACADE_UPDATE_H_

#include "sqlquery.hh"
#include "sqlexception.hh"
#include <algorithm>

namespace softeq
{
namespace db
{
/*!
    \brief Class for updating data in a specified table
*/
class UpdateQuery : public SqlConditionalQuery<UpdateQuery>
{
public:
    explicit UpdateQuery(const TableScheme &scheme);
};

namespace query
{
/*!
    \brief Forms an UPDATE query for the selected table
    By default, the structure field marked as PRIMARY_KEY is used as the search identifier.
    if WHERE is used then it overwrites the search condition
    \tparam <Struct> containing the schema of the database table
    \param[in] data A Struct filled with data to be update into a table. ATTEND: All fields passed in the
    structure are updated, including those that are not explicitly initialized by default
*/
template <typename Struct>
UpdateQuery update(const Struct &data)
{
    auto scheme = buildTableScheme<Struct>();
    std::vector<Cell> cells = scheme.cells();
    for (Cell &cell : cells)
    {
        cell.serialize(data);
    }

    Condition id;

    cells.erase(std::remove_if(cells.begin(), cells.end(),
                               [&](const Cell &cell) {
                                   if (cell.flags() == Cell::Flags::PRIMARY_KEY)
                                   {
                                       id = (Condition(cell) == cell.value());
                                       return true;
                                   }
                                   return false;
                               }),
                cells.end());

    UpdateQuery query(scheme);
    query.setCells(std::move(cells));
    query.where(id);

    return query;
}

/*!
    \brief Forms an UPDATE query for the selected table with specified fields.
    \tparam <Struct> containing the schema of the database table
    \param[in] fields fields to update (only these fields will be updated)
    \param[in] data A Struct filled with data to be update into a table
*/
template <typename Struct>
UpdateQuery update(std::initializer_list<CellMaker> fields, const Struct &data)
{
    if (fields.size() == 0)
    {
        throw SqlException("no columns to update");
    }

    std::vector<Cell> cells;
    for (const CellMaker &field : fields)
    {
        cells.emplace_back(field());
        cells.front().serialize(data);
    }

    UpdateQuery query(buildTableScheme<Struct>());
    query.setCells(std::move(cells));

    return query;
}

} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_UPDATE_H_
