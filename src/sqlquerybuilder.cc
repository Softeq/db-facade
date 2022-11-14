#include "sqlquerybuilder.hh"
#include "sqlexception.hh"

#include "createtable.hh"
#include "insert.hh"
#include "select.hh"
#include "update.hh"
#include "remove.hh"
#include "alter.hh"
#include "drop.hh"
#include "transaction.hh"

#include "condition.hh"
#include "cell.hh"

#include <tuple>
#include <iterator>
#include <sstream>
#include <ctime>
#include <cassert>
#include <set>
#include <cstring>
#include "constraints.hh"

namespace softeq
{
namespace db
{
Statement::Statement(const std::string &value)
{
    _expression.emplace_back(value);
}
Statement::Statement(const char *value)
{
    _expression.emplace_back(value);
}

Statement::Statement(std::vector<Token> &&from)
    : _expression(std::move(from))
{
}

std::string Statement::compose(const std::string &placeholderText) const
{
    std::stringstream ss;
    for (auto &token : _expression)
    {
        ss << (token.isValue() ? placeholderText : token.text());
    }
    return ss.str();
}

std::vector<SqlValue> Statement::parameters() const
{
    return Token::bindingParameters(_expression);
}

namespace internal
{
template <typename ElementT, typename SeparatorT, typename ConvertedElementT = ElementT>
class JoinWith
{
public:
    using Converter = const std::function<ConvertedElementT(ElementT)> &;

    JoinWith(const std::vector<ElementT> &elements, SeparatorT separator, SeparatorT empty, Converter convert)
        : _elements(elements)
        , _separator(separator)
        , _empty(empty)
        , _convert(convert)
    {
    }

    template <typename StreamT>
    friend StreamT &operator<<(StreamT &stream, const JoinWith &join)
    {
        if (join._elements.empty())
        {
            stream << join._empty;
        }
        else
        {
            for (auto iter = std::begin(join._elements); iter != std::end(join._elements); ++iter)
            {
                stream << join._convert(*iter);
                if (std::next(iter) != std::end(join._elements))
                {
                    stream << join._separator;
                }
            }
        }
        return stream;
    }

private:
    const std::vector<ElementT> &_elements;
    SeparatorT _separator;
    SeparatorT _empty;
    Converter _convert;
};

template <typename ElementT, typename SeparatorT, typename ConvertedElementT = ElementT>
static JoinWith<ElementT, SeparatorT, ConvertedElementT> join(
    const std::vector<ElementT> &elements, SeparatorT separator, SeparatorT empty = "",
    typename JoinWith<ElementT, SeparatorT, ConvertedElementT>::Converter convert = [](const ElementT &element) {
        return element;
    })
{
    return JoinWith<ElementT, SeparatorT, ConvertedElementT>(elements, separator, empty, convert);
}

} // namespace internal

SqlQueryStringBuilder::SqlQueryStringBuilder(CellRepresentation &cellRepr)
    : _cellRepr(cellRepr)
{
}

SqlQueryStringBuilder::~SqlQueryStringBuilder()
{
}

CellRepresentation &SqlQueryStringBuilder::cellRepr() const
{
    return _cellRepr;
}

std::vector<Token> SqlQueryStringBuilder::where(const Condition &condition)
{
    std::vector<Token> retval;

    if (condition.hasValue())
    {
        retval << " WHERE " << condition.tokens();
    }

    return retval;
}

std::vector<Token> SqlQueryStringBuilder::join(const std::vector<Join> &joins)
{
    std::vector<Token> tokens;

    if (!joins.empty())
    {
        for (const Join &join : joins)
        {
            tokens << " JOIN " << join.tokens();
        }
    }
    return tokens;
}

std::string SqlQueryStringBuilder::orderBy(const std::vector<OrderBy> &orderbys)
{
    if (!orderbys.empty())
    {
        std::stringstream ss;
        ss << " ORDER BY ";
        std::copy(orderbys.begin(), std::prev(orderbys.end()), std::ostream_iterator<OrderBy>(ss, ", "));
        ss << *std::prev(orderbys.end());
        return ss.str();
    }
    return {};
}

std::string SqlQueryStringBuilder::limit(const ResultLimit &limits) const
{
    std::stringstream ss;
    if (limits.defined())
    {
        ss << " LIMIT " << limits.rowsOffset << ", " << limits.rowsLimit;
    }
    return ss.str();
}

std::string SqlQueryStringBuilder::buildConstraints(const CreateTableQuery &query) const
{
    auto scheme = query.scheme();
    std::ostringstream os;
    if (!scheme.constraints().empty())
    {
        os << ", ";
        for (const auto &constraint : scheme.constraints())
        {
            os << constraint.get()->toString(*this, scheme);
        }
    }
    return os.str();
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class CreateTableQuery &query) const
{
    std::vector<Token> sql;

    sql << "CREATE TABLE IF NOT EXISTS " << query.table();

    if (!query.schemeSource().name().empty()) // use query to create table
    {
        std::vector<CellRepresentation::column_t> cols = cellRepr().columns(query);
        const auto &schemeSource = query.schemeSource();
        auto oldCols = cellRepr().columns(schemeSource.cells());
        cellRepr().fillMissingDefaults(oldCols, cols);
        // note that JOIN is not supported here while it is possible in SQL
        // it does not fit well with the concept of tables as c++ structs

        auto fields = cellRepr().fieldsWithCasts(cols);

        sql << " AS SELECT " << internal::join(fields, ", ") << " FROM " << schemeSource.name()
            << where(query.condition()) << orderBy(query.orderBys()) << ";";
    }
    else // create a table in a regular way
    {
        auto fields = cellRepr().fieldsWithDescr(cellRepr().columns(query));
        sql << "(" << internal::join(fields, ", ") << buildConstraints(query) << ");";
    }

    return {Statement(std::move(sql))};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class InsertQuery &query) const
{
    std::vector<Token> tokens;

    auto shortNames = cellRepr().fieldsShortNames(query.cells());
    auto values = cellRepr().values(query.cells());

    tokens << "INSERT INTO " << query.table() << " (" << internal::join(shortNames, ", ") << ") VALUES ("
           << internal::join(values, ", ") << ");";

    return {Statement{std::move(tokens)}};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class SelectQuery &query) const
{
    std::vector<Token> sql;
    auto selectFields = cellRepr().fieldsNames(cellRepr().columns(query));

    sql << "SELECT " << internal::join(selectFields, ", ", "*") << " FROM " << query.table() << join(query.joins())
        << where(query.condition()) << orderBy(query.orderBys()) << limit(query.limits()) << ";";

    return {Statement{std::move(sql)}};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class RemoveQuery &query) const
{
    std::vector<Token> sql;

    sql << "DELETE FROM " << query.table() << where(query.condition()) << ";";

    return {Statement{std::move(sql)}};
}

std::vector<Token> buildUpdateStatementValues(const std::vector<std::string> &names,
                                              const std::vector<SqlValue> &values)
{
    std::vector<Token> part;

    for (size_t i = 0; i < names.size(); ++i)
    {
        part << names[i] << " = " << values[i];

        if (i < names.size() - 1)
        {
            part << ", ";
        }
    }

    return part;
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class UpdateQuery &query) const
{
    auto columns = cellRepr().columns(query);
    auto names = cellRepr().fieldsShortNames(columns);
    auto values = cellRepr().values(columns);
    auto setStatement = buildUpdateStatementValues(names, values);

    std::vector<Token> sql;

    sql << "UPDATE " << query.table() << " SET " << setStatement << where(query.condition());

    return {Statement(std::move(sql))};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class DropQuery &query) const
{
    std::stringstream result;
    result << "DROP TABLE IF EXISTS " << query.table();

    return {Statement(result.str())};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class AlterQuery &query) const
{
    std::vector<Statement> statements;
    std::string tableName = query.table();

    for (const auto &action : query.alters())
    {
        if (action.type == TableScheme::NO_OP)
        {
            continue;
        }

        std::stringstream result;
        result << "ALTER TABLE ";
        // action types are essentially different ALTER TABLE* commands
        switch (action.type)
        {
        case TableScheme::NO_OP:
            break;
        case TableScheme::RENAME_TABLE:
            result << tableName << " RENAME TO " << action.table;
            break;
        case TableScheme::ADD_COLUMN:
            result << tableName << " ADD "
                   << internal::join(cellRepr().fieldsWithDescr(cellRepr().columns({action.cell})), ", ");
            break;
        case TableScheme::DROP_COLUMN:
            result << tableName << " DROP COLUMN " << action.cell.unqualifiedName();
            break;
        case TableScheme::RENAME_COLUMN:
            result << tableName << " RENAME COLUMN " << action.renameCell.first.unqualifiedName() << " TO "
                   << action.renameCell.second.unqualifiedName();
            break;
        }
        result << ";";

        statements.emplace_back(result.str());
    }

    return statements;
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class BeginTransactionQuery &) const
{
    return std::vector<Statement>{"BEGIN TRANSACTION;"};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class CommitTransactionQuery &) const
{
    return std::vector<Statement>{"COMMIT;"};
}

std::vector<Statement> SqlQueryStringBuilder::buildStatement(const class RollbackTransactionQuery &) const
{
    return std::vector<Statement>{"ROLLBACK;"};
}

// TODO: this method is called toString but relevant for constraints only. Probably worth renaming
std::string SqlQueryStringBuilder::toString(const class constraints::ForeignKeyConstraint &fk,
                                            const class TableScheme &scheme) const
{
    std::stringstream ss;
    ss << "FOREIGN KEY ( " << scheme.cell(fk.cell().offset()).unqualifiedName() << " ) REFERENCES "
       << fk.foreignCell().tableName() << " ( " << fk.foreignCell().unqualifiedName() << " ) ";
    for (const auto &trigger : fk.triggers())
    {
        ss << " " << trigger.first << " " << trigger.second;
    }

    return ss.str();
}

} // namespace db
} // namespace softeq
