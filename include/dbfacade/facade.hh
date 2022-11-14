#ifndef SOFTEQ_DBFACADE_FACADE_H_
#define SOFTEQ_DBFACADE_FACADE_H_

#include "connection.hh"

namespace softeq
{
namespace db
{
namespace
{
// C++11 does not support variadic lambdas and std::apply, so we are doing it this way
/*!
    \brief Helper class to deserialize a tuple of Structs or single Structs
 */
class DeserializerHelper
{
private:
    const std::string &_colName;
    int _colIndex;
    const std::vector<const char *> &_row;

    template <typename Struct>
    bool deserializeSingle(Struct &single)
    {
        auto cellp = buildTableScheme<Struct>().findCell(_colName);
        if (cellp.second)
        {
            cellp.first.template deserialize<Struct>(_row[_colIndex], single);
        }
        return cellp.second;
    }

public:
    /*!
        \brief Cleates a closure for the functor
     */
    DeserializerHelper(const std::string &colName, int colIndex, const std::vector<const char *> &row)
        : _colName(colName)
        , _colIndex(colIndex)
        , _row(row)
    {
    }

    // Methods of calling the functor for a single Struct and for std::tuple<Struct...>

    template <typename Struct, typename... Args>
    typename std::enable_if<!std::is_same<Struct, typename std::tuple<Args...>>::value, bool>::type
    operator()(Struct &single)
    {
        return deserializeSingle(single);
    }

    template <std::size_t I = 0, typename... Tp>
    typename std::enable_if<I == sizeof...(Tp), bool>::type operator()(std::tuple<Tp...> &)
    {
        return false;
    }

    template <std::size_t I = 0, typename... Tp>
        typename std::enable_if < I<sizeof...(Tp), bool>::type operator()(std::tuple<Tp...> &tuple)
    {
        return deserializeSingle(std::get<I>(tuple)) || operator()<I + 1, Tp...>(tuple);
    }
};
} // namespace

/*!
    \brief A proxy class responsible to running SELECT queries and converting data to C++ data
    containers.
 */
class DataRetriever
{
    const SqlQuery &_query;
    Connection::SPtr _connection;

    /*!
        \brief Implementation of data retrieval
        \tparam RowT Struct or std::tuple<Struct...>
     */
    template <typename RowT>
    std::vector<RowT> retrieve() // we may consider making it public
    {
        std::vector<RowT> result;

        auto parseFunc = [&result](const std::map<std::string, int> &header, const std::vector<const char *> &row) {
            RowT single;
            for (const auto &col : header)
            {
                if (!DeserializerHelper(col.first, col.second, row)(single))
                {
                    throw SqlException("unknown cell: " + col.first);
                }
            }
            result.emplace_back(std::move(single));
        };

        _connection->perform(_query, parseFunc);
        return result;
    }

public:
    /*!
        \brief Create a data retriever object that can be casted to vector of Struct or
        vertor of tuples of Structs.
        \param connection pointer to SQL connection
        \param query query to perform
     */
    DataRetriever(Connection::SPtr connection, const SqlQuery &query)
        : _query(query)
        , _connection(connection)
    {
    }

    /*!
        \brief Method converts the result of a database query into a vector<Struct> containing the received data.
        Please note that only those directly requested are valid data in the fields, all other fields contain garbage
        \throw Throws an SqlException on perform error.
    */
    template <
        typename Struct, typename... Args,
        typename std::enable_if<!std::is_same<Struct, typename std::tuple<Args...>>::value, void>::type * = nullptr>
    operator std::vector<Struct>()
    {
        return retrieve<Struct>();
    }

    /*!
        \brief Method converts the result of a database query into a vector<tuple<Struct...>> containing the received
       data. Please note that only those directly requested are valid data in the fields, all other fields contain
       garbage
    */
    template <typename... Single>
    operator std::vector<std::tuple<Single...>>()
    {
        return retrieve<std::tuple<Single...>>();
    }
};

/*!
    \brief Class provides a simple interface to sql-like databases
*/
class Facade
{
    /*!
        \brief Executes a query which begins a transaction
    */
    void beginTransaction();

    /*!
        \brief Executes a query which ends a transaction
        \param commit bool wheather we need to commit a transaction
    */
    void endTransaction(bool commit = true);

public:
    explicit Facade(Connection::SPtr connection)
        : _connection(connection)
    {
    }

    /*!
        \brief Delivers your generated request to the database.
        The method is intended for queries that do NOT require the return of DATA from the database.
        \param TypedQuery containing the generated database query
    */
    void execute(const SqlQuery &query) const;

    /*!
        \brief Calls execute method for each argument
        \param query queris to execute one by one
    */
    template <typename... QueryTs>
    void execute(const SqlQuery &query, QueryTs &&... args) const
    {
        execute(query);
        execute(std::forward<QueryTs>(args)...);
    }

    /*!
        \brief Executes its argument in a transacted way, i.e. all
        database queries called in transactionFunction will be executed
        inside a transaction.
        If transactionFunction returns true, the transaction will be commited,
        otherwise it will be rolled back.
        \param transactionFunction the function to execute.
    */
    void execTransaction(std::function<bool(Facade &)> transactionFunction);

    /*!
        \brief Executes its arguments in a transacted way and commits the transaction.
        \param query queries to execute inside a transaction
    */
    template <typename... QueryTs>
    void execTransaction(const SqlQuery &query, QueryTs &&... args)
    {
        execTransaction([&query, &args...](Facade &storage) {
            storage.execute(query, std::forward<QueryTs>(args)...);
            return true;
        });
    }

    /*!
        \brief Delivers your generated query to the database.
        The method is designed to RETURN DATA from the database.
        \param TypedQuery containing the generated database query
    */
    DataRetriever receive(const SqlQuery &query)
    {
        return DataRetriever(_connection, query);
    }

    /*!
        \brief Verify if actual table matches the scheme,
        Throws an exception if it does not.
        \tparam TableT the scheme we except the table to match,
    */
    template <typename TableT>
    void verifyScheme()
    {
        _connection->verifyScheme(buildTableScheme<TableT>());
    }

private:
    Connection::SPtr _connection;
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_FACADE_H_
