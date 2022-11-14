#ifndef SOFTEQ_DBFACADE_SQLQUERYBUILDER_H_
#define SOFTEQ_DBFACADE_SQLQUERYBUILDER_H_

#include <string>
#include <vector>
#include <sstream>

#include "sqlvalue.hh"
#include "orderby.hh"
#include "resultlimit.hh"
#include "join.hh"
#include "constraints.hh"
#include "cellrepresentation.hh"
#include "token.hh"

namespace softeq
{
namespace db
{
/*!
    \brief A class that represents a statement, essentially a sequence of tokens
*/
class Statement
{
public:
    /*!
        \brief Construct a statement out of a string
        \param the string
    */
    Statement(const std::string &value);

    /*!
        \brief Construct a statement out of a string (const char*)
        \param the string
    */
    Statement(const char *value);

    /*!
        \brief Construct a statement out of a vector of tokens
        \param the vector of tokens
    */
    Statement(std::vector<Token> &&from);

    /*!
        \brief Creates a string representation of a statement and places placeholder
        instead of values (to bind them)
        \param placeholderText placeholder text, "?" by default
        \returns the string representation
    */
    std::string compose(const std::string &placeholderText = "?") const;

    /*!
        \brief Returns a vector of SqlValue objects
        \returns a vector of SqlValue objects
    */
    std::vector<SqlValue> parameters() const;

private:
    std::vector<Token> _expression;
};

class SqlQueryStringBuilder
{
    CellRepresentation &_cellRepr;

protected:
    /*!
        \brief Compose a string representation for a condition and 'WHERE' clause
        if a condition is specified
        \param condition a condition
        \return a string representation for an SQL query string
    */
    static std::vector<Token> where(const Condition &condition);

    /*!
        \brief Conpose a string representation for 'JOIN' clause
        \param joins a vector of Join objects
        \return a string representation for an SQL query string
    */
    static std::vector<Token> join(const std::vector<Join> &joins);

    /*!
        \brief Conpose a string representation for 'ORDER BY' clause
        \param orderbys a vector of OrderBy objects
        \return a string representation for an SQL query string
    */
    static std::string orderBy(const std::vector<OrderBy> &orderbys);

    /*!
        \brief Conpose a string representation for 'LIMIT/OFFSET' clause
        \param limits a ResultLimit object
        \return a string representation for an SQL query string
    */
    virtual std::string limit(const ResultLimit &limits) const;

    std::string buildConstraints(const class CreateTableQuery &query) const;

public:
    explicit SqlQueryStringBuilder(CellRepresentation &cellRepr);
    CellRepresentation &cellRepr() const;
    virtual ~SqlQueryStringBuilder();

    virtual std::vector<Statement> buildStatement(const class CreateTableQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class InsertQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class SelectQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class RemoveQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class UpdateQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class AlterQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class DropQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class BeginTransactionQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class CommitTransactionQuery &query) const;
    virtual std::vector<Statement> buildStatement(const class RollbackTransactionQuery &query) const;

    virtual std::string toString(const constraints::ForeignKeyConstraint &fk, const class TableScheme &scheme) const;
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SQLQUERYBUILDER_H_
