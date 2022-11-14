#include "alter.hh"
#include "sqlquerybuilder.hh"

namespace softeq
{
namespace db
{
AlterQuery::AlterQuery(const softeq::db::TableScheme &scheme)
    : SerializableSqlQuery(scheme)
{
    setCells(scheme.cells());
}

AlterQuery &AlterQuery::renamingCell(const CellMaker &oldCell, const CellMaker &newCell)
{
    TableScheme::renameColumn(_alters, oldCell(), newCell());
    return *this;
}

const TableScheme::DiffActionItems &AlterQuery::alters() const
{
    return _alters;
}

} // namespace db
} // namespace softeq
