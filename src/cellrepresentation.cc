#include "cellrepresentation.hh"

#include <algorithm>
#include <sstream>
#include <set>
#include <ctime>
#include <chrono>

#include "sqlquery.hh"
#include "sqlexception.hh"

namespace softeq
{
namespace db
{
// Helper function for building an SQL expression
std::string CellRepresentation::type(std::size_t hash)
{
    // we should probably switch to templates and use something like  std::is_integral
    if (hash == typeid(int).hash_code() || hash == typeid(std::int64_t).hash_code() ||
        hash == typeid(std::int32_t).hash_code() || hash == typeid(std::uint32_t).hash_code())
    {
        return "INTEGER";
    }
    if (hash == typeid(std::string).hash_code())
    {
        return "TEXT";
    }
    if (hash == typeid(std::chrono::time_point<std::chrono::system_clock>).hash_code())
    {
        return "DATETIME";
    }
    throw SqlException("Unsupported type");
}

std::string CellRepresentation::field(const column_t &col) const
{
    return col.name;
}

std::string CellRepresentation::fieldShortName(const column_t &col) const
{
    return col.alias;
}

std::string CellRepresentation::fieldWithDescr(const column_t &col) const
{
    return col.name + " " + col.type + col.descr;
}

std::string CellRepresentation::typeToCastType(const std::string &typeName) const
{
    return typeName;
}

std::string CellRepresentation::fieldWithCasts(const column_t &col) const
{
    std::string entry;
    if (col.defval.type() != softeq::db::SqlValue::Subtype::Empty)
    {
        // CAST(value AS type) AS field_name
        entry = "CAST(" + col.defval.toString() + " AS " + typeToCastType(col.type) + ") AS " + col.name;
    }
    else
    {
        entry = col.name;
        if (col.alias != col.name)
        {
            entry += " AS " + col.alias;
        }
    }
    return entry;
}

std::vector<std::string> CellRepresentation::fields(const std::vector<column_t> &cols, FieldAction action) const
{
    std::vector<std::string> result;
    for (const column_t &col : cols)
    {
        result.emplace_back((this->*action)(col));
    }
    return result;
}

std::vector<std::string> CellRepresentation::fieldsNames(const std::vector<column_t> &cols) const
{
    return fields(cols, &CellRepresentation::field);
}

std::vector<std::string> CellRepresentation::fieldsShortNames(const std::vector<column_t> &cols) const
{
    return fields(cols, &CellRepresentation::fieldShortName);
}

std::vector<std::string> CellRepresentation::fieldsShortNames(const std::vector<Cell> &cells) const
{
    return fieldsShortNames(columns(cells));
}

std::vector<std::string> CellRepresentation::fieldsWithDescr(const std::vector<column_t> &cols) const
{
    return fields(cols, &CellRepresentation::fieldWithDescr);
}

std::vector<std::string> CellRepresentation::fieldsWithCasts(const std::vector<column_t> &cols) const
{
    return fields(cols, &CellRepresentation::fieldWithCasts);
}

std::vector<SqlValue> CellRepresentation::values(const std::vector<column_t> &cols) const
{
    std::vector<SqlValue> result;
    for (const column_t &col : cols)
    {
        result.emplace_back(col.val);
    }
    return result;
}

std::vector<SqlValue> CellRepresentation::values(const std::vector<Cell> &cells) const
{
    return values(columns(cells));
}

std::string CellRepresentation::description(const Cell &cell) const
{
    std::stringstream ss;
    std::uint32_t flags(cell.flags());

    if (flags & Cell::Flags::PRIMARY_KEY)
    {
        ss << " PRIMARY KEY";
    }
    if (flags & Cell::Flags::UNIQUE)
    {
        ss << " UNIQUE";
    }
    if (flags & Cell::Flags::AUTOINCREMENT)
    {
        ss << " AUTOINCREMENT";
    }
    if (!cell.isNullable())
    {
        ss << " NOT NULL";
    }
    if (flags & Cell::Flags::DEFAULT)
    {
        ss << " DEFAULT '" << cell.config().toString() << "'";
    }

    return ss.str();
}

CellRepresentation::column_t CellRepresentation::column(const Cell &cell) const
{
    column_t entry;

    entry.name = cell.name();
    entry.alias = cell.unqualifiedName(); // alias cannot be qualified
    entry.val = cell.value();
    entry.defval = cell.config();
    entry.type = type(cell.typeHash());
    entry.descr = description(cell);

    return entry;
}

std::vector<CellRepresentation::column_t> CellRepresentation::columns(const std::vector<Cell> &cells) const
{
    std::vector<column_t> result;
    result.reserve(cells.size());
    std::transform(cells.begin(), cells.end(), std::back_inserter(result),
                   [this](const Cell &cell) { return column(cell); });
    return result;
}

std::vector<CellRepresentation::column_t> CellRepresentation::columns(const SqlQuery &query) const
{
    return columns(query.cells());
}

void CellRepresentation::fillMissingDefaults(const std::vector<column_t> &srcCols, std::vector<column_t> &dstCols,
                                             const SqlValue &defval)
{
    // get column names from srcCols
    std::set<std::string> srcColNames;
    std::transform(srcCols.begin(), srcCols.end(), std::inserter(srcColNames, srcColNames.end()),
                   [](const column_t &column) { return column.name; });

    // find new columns in dstCols and fille their default value with NULL if it's not specified
    for (auto &col : dstCols)
    {
        if (srcColNames.count(col.name) == 0 && col.defval.type() == SqlValue::Subtype::Empty)
        {
            col.defval = defval;
        }
    }
}

} // namespace db
} // namespace softeq
