#include "tablescheme.hh"
#include "sqlexception.hh"

#include <algorithm>
#include <set>

namespace softeq
{
namespace db
{
TableScheme::TableScheme(const std::string &name, std::initializer_list<Cell> cells,
                         std::initializer_list<std::shared_ptr<constraints::BaseConstraint>> constraints)
    : _name(name)
    , _cells(std::move(cells))
    , _constraints(std::move(constraints))
{
}

TableScheme::TableScheme(const std::string &name, std::initializer_list<Cell> cells)
    : TableScheme(name, Cells(cells))
{
}

TableScheme::TableScheme(const std::string &name, Cells &&cells)
    : _name(name)
    , _cells(std::move(cells))
{
    // check ups
    size_t primaries = 0;
    for (const Cell &cell : _cells)
    {
        if (cell.name().empty())
        {
            throw SqlException("Column is unnamed in table " + this->name());
        }
        if (cell.flags() & Cell::PRIMARY_KEY)
        {
            ++primaries; // actually we do not have to count them, sqlite can do it as well
        }
    }
    if (primaries > 1)
    {
        throw SqlException("At least two primary keys were detected in table " + this->name());
    }
}

Cell TableScheme::cell(std::ptrdiff_t offset) const
{
    auto cellp = std::find_if(std::begin(_cells), std::end(_cells),
                              [&offset](const Cell &cell) { return cell.offset() == offset; });
    if (cellp != std::end(_cells))
    {
        return *cellp;
    }
    throw SqlException("not declared");
}

std::pair<Cell, bool> TableScheme::findCell(const std::string &name) const
{
    auto cellp =
        std::find_if(std::begin(_cells), std::end(_cells), [&name](const Cell &cell) { return cell.name() == name; });
    if (cellp != std::end(_cells))
    {
        return {*cellp, true};
    }
    else
    {
        return {Cell(nullptr), false};
    }
}

namespace
{
/*!
    \brief Finds cells that are present in the first scheme but missing in the second
    \param lStruct the first scheme
    \param rStruct the second scheme
    \return a vector of missing cells
*/
std::vector<Cell> getMissingCells(const TableScheme &lScheme, const TableScheme &rScheme)
{
    struct cellLess
    {
        bool operator()(const Cell &lhs, const Cell &rhs) const
        {
            return lhs.name() < rhs.name();
        };
    };

    auto rCells = rScheme.cells();
    auto lCells = lScheme.cells();

    std::set<Cell, cellLess> rSet{rCells.begin(), rCells.end()};
    std::set<Cell, cellLess> lSet{lCells.begin(), lCells.end()};

    std::vector<Cell> diff;

    // add columns

    // elements that are present in the first (left) set, but not in the second (right) one
    std::set_difference(std::begin(lSet), std::end(lSet), std::begin(rSet), std::end(rSet), std::back_inserter(diff),
                        cellLess());

    return diff;
}

/*!
    \brief Converts cells to action using conversion function
    \param cells cells
    \param convertFunction conversion function
    \return a vector of conversion steps
*/
std::vector<TableScheme::DiffActionItem>
convertCellsToActions(const std::vector<Cell> &cells, std::function<TableScheme::DiffActionItem(Cell)> convertFunction)
{
    TableScheme::DiffActionItems resultActions;
    resultActions.reserve(cells.size());
    std::transform(std::begin(cells), std::end(cells), std::back_inserter(resultActions), convertFunction);
    return resultActions;
}

/*!
    \brief Converts cells to ADD COLUMN action
    \param cells cells
    \return a vector of conversion steps
*/
TableScheme::DiffActionItems convertToAddAction(const std::vector<Cell> &cells)
{
    return convertCellsToActions(cells, TableScheme::DiffActionItem::addColumn);
}

/*!
    \brief Converts cells to DROP COLUMN action
    \param cells cells
    \return a vector of conversion steps
*/
TableScheme::DiffActionItems convertToDropAction(const std::vector<Cell> &cells)
{
    return convertCellsToActions(cells, TableScheme::DiffActionItem::dropColumn);
}
} // namespace

TableScheme::DiffActionItems TableScheme::generateConversionSteps(const TableScheme &toScheme) const
{
    const TableScheme &fromScheme = *this;
    DiffActionItems diff;

    auto cellsToAdd = convertToAddAction(getMissingCells(toScheme, fromScheme));
    auto cellsToDrop = convertToDropAction(getMissingCells(fromScheme, toScheme));

    // drop columns
    diff.insert(std::end(diff), std::begin(cellsToDrop), std::end(cellsToDrop));

    // add columns
    diff.insert(std::end(diff), std::begin(cellsToAdd), std::end(cellsToAdd));

    // rename table
    if (fromScheme.name() != toScheme.name())
    {
        diff.push_back(DiffActionItem::renameTable(toScheme.name()));
    }

    return diff;
}

void TableScheme::renameColumn(TableScheme::DiffActionItems &items, const Cell &from, const Cell &to)
{
    auto drop = std::find_if(std::begin(items), std::end(items), [&from](const DiffActionItem &item) {
        return item.type == DROP_COLUMN && item.cell.unqualifiedName() == from.unqualifiedName();
    });
    if (drop == std::end(items))
    {
        throw SqlException("no such column (src): " + from.name());
    }

    auto add = std::find_if(std::begin(items), std::end(items), [&to](const DiffActionItem &item) {
        return item.type == ADD_COLUMN && item.cell.unqualifiedName() == to.unqualifiedName();
    });

    if (add == std::end(items))
    {
        throw SqlException("no such column (dst): " + to.name());
    }

    *drop = TableScheme::DiffActionItem::renameColumn(from, to);
    *add = TableScheme::DiffActionItem::noop();
}

// Action constructors

TableScheme::DiffActionItem TableScheme::DiffActionItem::noop()
{
    return {NO_OP, Cell(nullptr), {}, {Cell(nullptr), Cell(nullptr)}};
}

TableScheme::DiffActionItem TableScheme::DiffActionItem::renameTable(const std::string &newName)
{
    return {RENAME_TABLE, Cell(nullptr), newName, {Cell(nullptr), Cell(nullptr)}};
}

TableScheme::DiffActionItem TableScheme::DiffActionItem::addColumn(const Cell &cell)
{
    return {ADD_COLUMN, cell, {}, {Cell(nullptr), Cell(nullptr)}};
}

TableScheme::DiffActionItem TableScheme::DiffActionItem::dropColumn(const Cell &cell)
{
    return {DROP_COLUMN, cell, {}, {Cell(nullptr), Cell(nullptr)}};
}

TableScheme::DiffActionItem TableScheme::DiffActionItem::renameColumn(const Cell &from, const Cell &to)
{
    return {RENAME_COLUMN, Cell(nullptr), {}, {from, to}};
}

} // namespace db
} // namespace softeq
