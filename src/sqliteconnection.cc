#include "sqliteconnection.hh"
#include "sqliteexception.hh"
#include "alter.hh"
#include "select.hh"

#include <sqlite3.h>
#include <iostream>
#include <algorithm>
#include <map>

namespace softeq
{
namespace db
{
SqliteQueryStringBuilder::SqliteQueryStringBuilder(CellRepresentation &cellRepr)
    : SqlQueryStringBuilder(cellRepr)
{
}

std::string SqliteQueryStringBuilder::limit(const ResultLimit &limits) const
{
    std::stringstream ss;
    if (limits.defined())
    {
        ss << " LIMIT " << limits.rowsOffset << ", ";
        if (limits.finite())
        {
            ss << limits.rowsLimit;
        }
        else // infinite
        {
            ss << -1;
        }
    }
    return ss.str();
}

std::vector<Statement> SqliteQueryStringBuilder::buildStatement(const class AlterQuery &query) const
{
    // Note: we may need to re-write this method after adding transactions
    // because we use it here

    // Sqlite cannot drop columns, so we need to copy a table in order to alter table.
    const auto &alters = query.alters();
    if (std::find_if(alters.begin(), alters.end(), [](const TableScheme::DiffActionItem &action) {
            return action.type == TableScheme::DROP_COLUMN;
        }) == alters.end())
    {
        // we do not need to drop columns
        return SqlQueryStringBuilder::buildStatement(query);
    }

    std::vector<Statement> ret;

    // probably creating a transaction is a nice thing since
    // alter is not an atomic operation in sqlite
    ret.emplace_back("BEGIN TRANSACTION;");

    // Create a copy of the table with field we need
    std::stringstream statement;
    statement << "CREATE TABLE tmp_" << query.table() << " AS SELECT ";

    // process alter operations
    auto cols = cellRepr().columns(query.cells());
    auto newTableName = query.table();
    for (const auto &action : alters)
    {
        switch (action.type)
        {
        case TableScheme::NO_OP:
            break;
        case TableScheme::RENAME_TABLE:
            newTableName = action.table;
            break;
        case TableScheme::ADD_COLUMN:
        {
            auto col = cellRepr().column(action.cell);
            if (col.defval.type() == SqlValue::Subtype::Empty)
            {
                // it's a new column, it must have default
                col.defval = SqlValue::Null();
            }
            cols.push_back(col);
            break;
        }
        case TableScheme::DROP_COLUMN:
            // if std::find_if returns cols.end(), it will mean we have messed up with calculating operations
            cols.erase(std::find_if(cols.begin(), cols.end(), [&action](const CellRepresentation::column_t &col) {
                return col.name == action.cell.unqualifiedName();
            }));
            break;
        case TableScheme::RENAME_COLUMN:
        {
            auto foundCol = std::find_if(cols.begin(), cols.end(), [&action](const CellRepresentation::column_t &col) {
                return col.name == action.renameCell.first.unqualifiedName();
            });
            foundCol->alias = action.renameCell.second.unqualifiedName();
            break;
        }
        }
    }
    auto newFields = cellRepr().fieldsWithCasts(cols);
    std::copy(newFields.begin(), std::prev(newFields.end()), std::ostream_iterator<std::string>(statement, ", "));
    statement << *std::prev(newFields.end()) << " FROM " << query.table() << ";";
    ret.emplace_back(statement.str());

    // Drop the old table
    statement = std::stringstream();
    statement << "DROP TABLE " << query.table() << ";";
    ret.emplace_back(statement.str());

    // Rename tmp_* to the original name
    statement = std::stringstream();
    statement << "ALTER TABLE tmp_" << query.table() << " RENAME TO " << newTableName << ";";
    ret.emplace_back(statement.str());

    // End transaction
    ret.emplace_back("COMMIT;");

    return ret;
}

SqliteConnection::SqliteConnection(const std::string &dbName)
{
    int ec = sqlite3_open(dbName.c_str(), &_db);
    if (ec != SQLITE_OK)
    {
        throw SqliteException("Error creating sqlite3 connection", ec);
    }
    enableForeignKeySupport();
    enableWaitingOnBusy();
}

SqliteConnection::~SqliteConnection()
{
    sqlite3_close(_db);
}

void SqliteConnection::verifyScheme(const TableScheme &scheme)
{
    std::map<std::string, Cell> expectedCells;
    for (auto &&cell : scheme.cells())
    {
        expectedCells[cell.name()] = std::move(cell);
    }

    // Example of "PRAGMA table_info('tablename')" result:
    // cid|name|type|notnull|dflt_value|pk
    // 0|id|integer|1||1
    // 1|name|text|0||0
    auto processRow = [this, &expectedCells](const std::map<std::string, int> &header,
                                             const std::vector<const char *> &row) {
        auto fixNull = [](const char *ptr) { return ptr ? ptr : ""; };

        constexpr const char *nameColumn = "name";
        constexpr const char *typeColumn = "type";
        constexpr const char *defaultColumn = "dflt_value";
        constexpr const char *notnullColumn = "notnull";
        constexpr const char *primarykeyColumn = "pk";
        constexpr const char *trueValue = "1";

        // info we get from actual (existing) table
        const std::string name = fixNull(row[header.at(nameColumn)]);
        const std::string type = fixNull(row[header.at(typeColumn)]);
        const std::string defval = fixNull(row[header.at(defaultColumn)]);
        const std::string isPK = fixNull(row[header.at(primarykeyColumn)]);
        const std::string notNull = fixNull(row[header.at(notnullColumn)]);
        std::uint32_t flags = 0;
        bool isNullable = false;
        if (notNull != trueValue)
        {
            isNullable = true;
        }
        if (isPK == trueValue)
        {
            flags |= Cell::PRIMARY_KEY;
        }

        auto cellp = expectedCells.find(name);
        if (cellp == std::end(expectedCells))
        {
            throw SqlException("Column '" + name + "' does not exist in the scheme");
        }

        Cell &cell = cellp->second;

        std::string expectedType = queryBuilder().cellRepr().type(cell.typeHash());
        if (type != expectedType)
        {
            throw SqlException("Type " + type + " of column '" + name + "' does not match type " + expectedType +
                               " in scheme");
        }

        auto cellConfig = cell.config();

        if (defval != cellConfig.toString())
        {
            std::string exception = "Default value '" + defval + "' of column '" + name +
                                    "' does not match expected value " + cellConfig.toString();

            throw SqlException("Default value '" + defval + "' of column '" + name +
                               "' does not match expected value " + cellConfig.toString());
        }

        if (isNullable != cell.isNullable()) // check changes for nullable flag
        {
            throw SqlException("The value of the nullable flag for column '" + name + "' has value (" +
                               std::to_string(isNullable) + ") and does not match expected (" +
                               std::to_string(cell.isNullable()) + ")");
        }

        // NOTE: flag check does not check equivalence of flags, it check whether actual flags are included into
        // expected ones. IMO it is more reasonable considering we can't get all flags.
        // Moreover, probably PK should not be checked either since it can be set using constraints.
        bool pkCheck = (isPK == trueValue) == !!(cell.flags() & Cell::PRIMARY_KEY);
        if (!pkCheck) // check only PRIMARY KEY
        {
            throw SqlException("Parameters (" + std::to_string(flags) + ") of column '" + name +
                               "' do not match expected parameters (" + std::to_string(cell.flags()) + ")");
        }

        expectedCells.erase(cellp); // checked - erase
    };

    performImpl({"PRAGMA table_info('" + scheme.name() + "');"}, processRow);

    if (!expectedCells.empty())
    {
        throw SqlException("Column '" + std::begin(expectedCells)->first + "' from scheme does not exist in the table");
    }
}

namespace
{
/*!
    \brief Returns a header for Sqlite statement
    \param statement Sqlite statement
    \returns column -> index map
*/
std::map<std::string, int> header(sqlite3_stmt *stmt)
{
    std::map<std::string, int> columnsMap;
    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i)
    {
        auto colName = static_cast<const char *>(sqlite3_column_name(stmt, i));
        columnsMap[colName] = i;
    }
    return columnsMap;
}

/*!
    \brief Executes a Sqlite statement with binding parameters
    \param statement Sqlite statement
    \param params a vector of SqlValue objects
    \param fn parse function
*/
void executeSql(sqlite3 *db, const char *sql, const std::vector<SqlValue> &params,
                const SqliteConnection::parseFunc &fn)
{
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        throw SqliteException(sqlite3_errmsg(db), sql, rc);
    }

    std::unique_ptr<sqlite3_stmt, void (*)(sqlite3_stmt *)> stmtGuard(
        stmt, [](sqlite3_stmt *statement) { sqlite3_finalize(statement); });

    if (params.size())
    {
        for (int i = 0; i < static_cast<int>(params.size()); ++i)
        {
            switch (params[i].type())
            {
            case SqlValue::Subtype::Null:
                rc = sqlite3_bind_null(stmt, i + 1);
                break;
            case SqlValue::Subtype::String:
                rc = sqlite3_bind_text(stmt, i + 1, params[i].strValue().c_str(),
                                       static_cast<int>(params[i].strValue().length()), nullptr);
                break;
            case SqlValue::Subtype::Integer:
                rc = sqlite3_bind_int64(stmt, i + 1, params[i].intValue());
                break;
            default:
                throw SqlException("unimplemented type");
                break;
            }

            if (rc != SQLITE_OK)
            {
                throw SqliteException(sqlite3_errmsg(db), rc);
            }
        }
    }

    if (rc != SQLITE_OK)
    {
        throw SqliteException(sqlite3_errmsg(db), rc);
    }

    rc = SQLITE_ROW;
    std::map<std::string, int> columnsMap;
    while (rc == SQLITE_ROW)
    {
        std::vector<const char *> row;
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            if (columnsMap.empty())
            {
                columnsMap = header(stmt);
            }

            int colCount = sqlite3_column_count(stmt);
            row.resize(colCount);
            for (int i = 0; i < colCount; ++i)
            {
                row[i] = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            }

            fn(columnsMap, row);
        }
    }
    if (rc != SQLITE_DONE)
    {
        throw SqliteException(rc);
    }
}
} // namespace

void SqliteConnection::performImpl(const std::vector<Statement> &statements, const parseFunc &fn)
{
    for (const Statement &statement : statements)
    {
        executeSql(_db, statement.compose().c_str(), statement.parameters(), fn);
    }
}

SqlQueryStringBuilder &SqliteConnection::queryBuilder()
{
    return _builder;
}

void SqliteConnection::enableForeignKeySupport()
{
    performImpl({"PRAGMA foreign_keys = ON;"}, nullptr);
}

void SqliteConnection::enableWaitingOnBusy()
{
    // instead of failing parallel request, sqlite will call the callback
    // and "return 1" means it should try again
    sqlite3_busy_handler(
        _db, [](void *, int) { return 1; }, nullptr);
}

} // namespace db
} // namespace softeq
