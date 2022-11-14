#include "mysqlquerybuilder.hh"
#include "mysqlexception.hh"
#include <cassert>

namespace softeq
{
namespace db
{
namespace mysql
{

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v)
{
    if (!v.empty())
    {
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out.seekp(-2, std::ios_base::cur);
    }
    else
    {
        out << "*";
    }

    out.flush();
    return out;
}

MySqlQueryStringBuilder::MySqlQueryStringBuilder(CellRepresentation &cellRepr)
    : SqlQueryStringBuilder(cellRepr)
{
}

std::string adjustQueryTerminationCharacter(const std::string &query)
{
    if (query.substr(query.length() - 1) == ",")
    {
        std::string fixQuery = query;
        fixQuery.pop_back();
        fixQuery.push_back(';');
        return fixQuery;
    }

    return query;
}

std::vector<Statement> MySqlQueryStringBuilder::buildStatement(const class AlterQuery &query) const
{
    // TODO: https://jira.softeq.com/browse/EMBEDU-411

    using namespace db;

    std::stringstream result;

    result << "ALTER TABLE " << query.table();

    for (const auto &action : query.alters())
    {
        if (action.type == TableScheme::NO_OP)
        {
            continue;
        }
        // action types are essentially diffrent ALTER TABLE* commands
        switch (action.type)
        {
        case TableScheme::NO_OP:
            break;
        case TableScheme::RENAME_TABLE:
            result << " RENAME TO " << action.table;
            break;
        case TableScheme::ADD_COLUMN:
            result << " ADD " << cellRepr().fieldsWithDescr(cellRepr().columns({action.cell}));
            break;
        case TableScheme::DROP_COLUMN:
            result << " DROP " << action.cell.unqualifiedName();
            break;
        case TableScheme::RENAME_COLUMN:
            result << " RENAME COLUMN " << action.renameCell.first.unqualifiedName() << " TO "
                   << action.renameCell.second.unqualifiedName();
            break;
        default:
            assert("Action types passed an element that does not have a handler in the case!");
            break;
        }
        result << ",";
    }

    return {adjustQueryTerminationCharacter(result.str())};
}

std::vector<Statement> MySqlQueryStringBuilder::buildStatement(const class BeginTransactionQuery &) const
{
    return {Statement("START TRANSACTION; ")};
}

std::string MySqlCellRepresentation::typeToCastType(const std::string &typeName) const
{
    return typeName == "INTEGER" ? "SIGNED" : CellRepresentation::typeToCastType(typeName);
}

std::string MySqlCellRepresentation::description(const Cell &cell) const
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
        ss << " AUTO_INCREMENT";
    }
    if (!(cell.isNullable()))
    {
        ss << " NOT NULL";
    }
    if (flags & Cell::Flags::DEFAULT)
    {
        ss << " DEFAULT " << cell.config().toString();
    }

    return ss.str();
}

} // namespace mysql
} // namespace db
} // namespace softeq
