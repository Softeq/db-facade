#include "mysqlconnection.hh"
#include "mysqlexception.hh"

#include <iostream>
#include <algorithm>
#include <map>

namespace softeq
{
namespace db
{
namespace mysql
{
#include <mysql/mysql.h>

MySqlConnection::MySqlConnection(const std::string &host, const int port, const std::string &userName,
                                 const std::string &password, const std::string &database)
    : _session(mysql_init(nullptr))
{
    if (_session == nullptr)
    {
        throw MySqlException("Failed to initialize mysql client");
    }

    if (!mysql_real_connect(_session, host.c_str(), userName.c_str(), password.c_str(), database.c_str(), port, nullptr,
                            0))
    {
        mysql_close(_session);
        throw MySqlException(mysql_error(_session));
    }
}

MySqlConnection::~MySqlConnection()
{
    if (_session != nullptr)
    {
        mysql_close(_session);
    }
}

namespace
{
constexpr static size_t defaultCellBufferSize = 100; /// Default return cell buffer size. if value is bigger
                                                     /// implementation will allocate separate buffer for storing value

// Smart pointers for MySQL resource handling

struct MysqlResultDeleter
{
    void operator()(MYSQL_RES *res)
    {
        if (res)
        {
            mysql_free_result(res);
        }
    }
};
using MysqlResultPtr = std::unique_ptr<MYSQL_RES, MysqlResultDeleter>;

struct MysqlStatementDeleter
{
    void operator()(MYSQL_STMT *stmt)
    {
        if (stmt)
        {
            mysql_stmt_close(stmt);
        }
    };
}; // namespace
using StatementPtr = std::unique_ptr<MYSQL_STMT, MysqlStatementDeleter>;

/*!
    \brief Creates bind parameter based on SqlValue
    \param param SqlValue
*/
MYSQL_BIND makeBind(SqlValue &param)
{
    MYSQL_BIND bind{};
    switch (param.type())
    {
    case SqlValue::Subtype::Null:
        bind.buffer_type = MYSQL_TYPE_NULL;
        break;

    case SqlValue::Subtype::Integer:
        bind.buffer_type = MYSQL_TYPE_LONGLONG;
        bind.buffer = &param.intValue();
        break;

    case SqlValue::Subtype::String:
        bind.buffer_type = MYSQL_TYPE_STRING;
        bind.buffer = const_cast<char *>(param.strValue().c_str());
        bind.buffer_length = param.strValue().length();
        break;

    default:
        throw SqlException("unsupported bind parameter");
        break;
    }
    return bind;
}

/*!
    \brief Bonds parameters to MySQL statement
    \param statement MySQL statement
    \param parameters parameters to bind
*/
void bindParameters(MYSQL_STMT *statement, std::vector<SqlValue> &parameters)
{
    if (parameters.size() > 0)
    {
        std::vector<MYSQL_BIND> parameterBinds;
        std::transform(std::begin(parameters), std::end(parameters), std::back_inserter(parameterBinds), makeBind);

        bool bindFailed = mysql_stmt_bind_param(statement, parameterBinds.data());
        if (bindFailed)
        {
            throw MySqlException(mysql_stmt_error(statement));
        }
    }
}

/*!
    \brief Returns a header for MySQL statement 
    \param statement MySQL statement
    \returns column -> index map
*/
std::map<std::string, int> header(MYSQL_STMT *statement)
{
    std::map<std::string, int> columnsMap;

    MysqlResultPtr resultStatement(mysql_stmt_result_metadata(statement));
    if (!resultStatement)
    {
        throw MySqlException(mysql_stmt_error(statement));
    }

    auto resultColumnCount = mysql_num_fields(resultStatement.get());

    MYSQL_FIELD *fields = mysql_fetch_fields(resultStatement.get());
    if (fields == nullptr)
    {
        throw MySqlException(mysql_stmt_error(statement));
    }

    for (unsigned int i = 0; i < resultColumnCount; i++)
    {
        columnsMap[fields[i].name] = i;
    }

    return columnsMap;
}

// this structure contains output buffers for mysql to fill
struct FetchedColumnBuffers
{
    unsigned long length;
    bool isNull;
    char data[defaultCellBufferSize + 1] = {0};
};

/*!
    \brief Fetches a row from executed MySQL statement. If buffers specified by 
    resultColumns arguments are not big enough, the data should be re-fetched manually.
    \param statement MySQL statement
    \param resultColumns a vector of buffers used when binding values
    \param rowVector output vector of string values
    \returns true is there are more rows to fetch, false otherwise
*/
bool fetchRow(MYSQL_STMT *statement, const std::vector<FetchedColumnBuffers> &resultColumns,
              std::vector<const char *> &rowVector)
{
    auto resultColumnCount = rowVector.size();

    int res = mysql_stmt_fetch(statement);

    if (res == MYSQL_NO_DATA)
    {
        return false; // wo do not have more rows
    }

    if (res != 0 && res != MYSQL_DATA_TRUNCATED)
    {
        throw MySqlException(mysql_stmt_error(statement));
    }

    // vector of buffers for column values that need reallocation
    std::vector<std::vector<char>> rowBuffer(resultColumnCount);

    // go over the columns, check if the data is not truncated and store in rowVector
    for (unsigned i = 0; i < resultColumnCount; ++i)
    {
        if (resultColumns[i].isNull)
        {
            rowVector[i] = nullptr;
        }
        else
        {
            // fetch again single column if we find that our buffer was not enough
            if (resultColumns[i].length > sizeof(resultColumns[i].data) - 1)
            {
                MYSQL_BIND cellBind{};
                rowBuffer.emplace_back(resultColumns[i].length + 1, 0);

                cellBind.buffer_type = MYSQL_TYPE_STRING;
                cellBind.buffer = rowBuffer.back().data();
                cellBind.buffer_length = resultColumns[i].length + 1;

                if (mysql_stmt_fetch_column(statement, &cellBind, i, 0) != 0)
                {
                    throw MySqlException(mysql_stmt_error(statement));
                }

                rowVector[i] = rowBuffer.back().data();
            }
            else
            {
                rowVector[i] = resultColumns[i].data;
            }
        }
    }
    return true; // continue with the next row
}

/*!
    \brief Binds result buffers to the statement
    \param statement MySQL statement
    \param resultColumns vector of result column buffers to store information to
*/
void bindResults(MYSQL_STMT *statement, std::vector<FetchedColumnBuffers> &resultColumns)
{
    auto resultColumnCount = resultColumns.size();
    std::vector<MYSQL_BIND> resultBinds(resultColumnCount);

    for (size_t i = 0; i < static_cast<size_t>(resultColumnCount); i++)
    {
        resultBinds[i].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[i].buffer = resultColumns[i].data;
        resultBinds[i].buffer_length = sizeof(resultColumns[i].data);
        resultBinds[i].length = &resultColumns[i].length;
        resultBinds[i].is_null = &resultColumns[i].isNull;
    }

    if (mysql_stmt_bind_result(statement, resultBinds.data()) != 0)
    {
        throw MySqlException(mysql_stmt_error(statement));
    }
}

} // namespace

void MySqlConnection::performImpl(const std::vector<Statement> &queries, const parseFunc &fn)
{
    for (auto line : queries) // NOTE: if parsing of some statement fails, the rest will not be executed. This is
                              // particularly bad for transactions
    {
        std::string sqlText = line.compose();

        // MySQL has issue when executing "START TRANSACTION" expression as statement so we do it using mysql_query
        // The error is "This command is not supported in the prepared statement protocol yet"
        if (sqlText == "START TRANSACTION; ")
        {
            int result = mysql_query(_session, sqlText.c_str());
            if (result != 0)
            {
                throw MySqlException(mysql_error(_session), sqlText);
            }
            continue;
        }

        // prepare statement
        StatementPtr statement(mysql_stmt_init(_session));
        if (!statement.get())
        {
            throw MySqlException(mysql_error(_session), sqlText);
        }

        if (mysql_stmt_prepare(statement.get(), sqlText.c_str(), sqlText.length()) != 0)
        {
            throw MySqlException(mysql_stmt_error(statement.get()), sqlText);
        }

        // bind parameters if any
        auto parameters = line.parameters();
        bindParameters(statement.get(), parameters);

        // execute statement
        if (mysql_stmt_execute(statement.get()) != 0)
        {
            throw MySqlException(mysql_stmt_error(statement.get()), sqlText);
        }

        // fetch and pass the result if any
        if (fn)
        {
            // get metadata
            std::map<std::string, int> columnsMap = header(statement.get());
            auto columnCount = columnsMap.size();

            // bind result
            std::vector<FetchedColumnBuffers> resultColumns(columnCount);
            bindResults(statement.get(), resultColumns);

            // fetch rows
            std::vector<const char *> rowVector(columnCount);
            while (fetchRow(statement.get(), resultColumns, rowVector))
            {
                fn(columnsMap, rowVector); // submit results to fn
            }
        }
    }
}

MySqlQueryStringBuilder &MySqlConnection::queryBuilder()
{
    return _builder;
}

void MySqlConnection::verifyScheme(const TableScheme &scheme)
{
    // TODO: it differs in some points from the implementation in sqlite, but looks similar in its structure. It's
    // probably worth thinking about removing code duplication
    std::map<std::string, Cell> expectedCells;
    for (auto &&cell : scheme.cells())
    {
        expectedCells[cell.name()] = std::move(cell);
    }

    // Example of "describe 'tablename'" result:
    // +-------+----------+------+-----+---------+----------------+
    // | Field | Type     | Null | Key | Default | Extra          |
    // +-------+----------+------+-----+---------+----------------+
    // | id    | int      | NO   | PRI | NULL    | auto_increment |
    // | name  | text     | NO   |     | NULL    |                |
    // | age   | int      | YES  |     | NULL    |                |
    // | date  | datetime | YES  |     | NULL    |                |
    // +-------+----------+------+-----+---------+----------------+

    auto processRow = [this, &expectedCells](const std::map<std::string, int> &header,
                                             const std::vector<const char *> &row) {
        auto fixNull = [](const char *ptr) { return ptr ? ptr : ""; };

        constexpr const char *nameColumn = "Field";
        constexpr const char *typeColumn = "Type";
        constexpr const char *defaultColumn = "Default";
        constexpr const char *nullColumn = "Null";
        constexpr const char *primarykeyColumn = "Key";
        constexpr const char *trueValue = "YES";
        constexpr const char *primarykeyValue = "PRI";

        auto iequal = [](const std::string &s, const std::string &t) {
            if (s.length() != t.length())
            {
                return false;
            }

            for (size_t i = 0; i < s.length(); ++i)
            {
                if (std::tolower(s[i]) != std::tolower(t[i]))
                {
                    return false;
                }
            }

            return true;
        };

        // info we get from actual (existing) table
        const std::string name = fixNull(row[header.at(nameColumn)]);
        const std::string type = fixNull(row[header.at(typeColumn)]);
        const std::string defval = fixNull(row[header.at(defaultColumn)]);
        const std::string isPK = fixNull(row[header.at(primarykeyColumn)]);
        const std::string isNull = fixNull(row[header.at(nullColumn)]);

        std::uint32_t flags = 0;
        if (isPK == primarykeyValue)
        {
            flags |= Cell::PRIMARY_KEY;
        }

        bool isNullable = false;
        if (isNull == trueValue) // primary key is automatically not null
        {
            isNullable = true;
        }

        auto cellp = expectedCells.find(name);
        if (cellp == std::end(expectedCells))
        {
            throw SqlException("Column '" + name + "' does not exist in the scheme");
        }

        Cell &cell = cellp->second;

        std::string expectedType = queryBuilder().cellRepr().type(cell.typeHash());
        if (expectedType == "INTEGER")
        {
            expectedType = "int";
        }
        if (!iequal(type, expectedType))
        {
            throw SqlException("Type " + type + " of column '" + name + "' does not match type " + expectedType +
                               " in scheme");
        }

        if (defval != cell.config().toString() &&
            !(defval == "<null>" && cell.config().type() == SqlValue::Subtype::Empty))
        {
            throw SqlException("Default value '" + defval + "' of column '" + name +
                               "' does not match expected value " + cell.config().toString());
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
        bool pkCheck = (isPK == primarykeyValue) == !!(cell.flags() & Cell::PRIMARY_KEY);
        if (!pkCheck) // check only NOT NULL and PRIMARY KEY
        {
            throw SqlException("Parameters (" + std::to_string(flags) + ") of column '" + name +
                               "' do not match expected parameters (" + std::to_string(cell.flags()) + ")");
        }

        expectedCells.erase(cellp); // checked - erase
    };

    std::string expression = "DESCRIBE " + scheme.name() + ";";

    performImpl({Statement(expression)}, processRow);

    if (!expectedCells.empty())
    {
        throw SqlException("Column '" + std::begin(expectedCells)->first + "' from scheme does not exist in the table");
    }
}

} // namespace mysql
} // namespace db
} // namespace softeq
