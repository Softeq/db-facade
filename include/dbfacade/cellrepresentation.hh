#ifndef SOFTEQ_DBFACADE_CELLREPRESENTATION_H_
#define SOFTEQ_DBFACADE_CELLREPRESENTATION_H_

#include <string>

#include "sqlvalue.hh"
#include "cell.hh"

namespace softeq
{
namespace db
{
class CellRepresentation
{
public:
    // Helper struct
    struct column_t
    {
        std::string name;
        std::string type;
        std::string descr;
        SqlValue val;
        SqlValue defval;
        std::string alias;
    };

protected:
    using FieldAction = std::string (CellRepresentation::*)(const column_t &) const;

    // Helper values for fields method. See below for documentation.

    virtual std::string field(const column_t &col) const;
    virtual std::string fieldShortName(const column_t &col) const;
    virtual std::string fieldWithDescr(const column_t &col) const;
    virtual std::string fieldWithCasts(const column_t &col) const;

    /*!
        \brief Create a vector of string representing a vector of column_t objects.
        \param cols a vector of column_t objects
        \param action a function which needs to be applied to the fields
            e.g. field to display only column name,
            fieldWithDescr if a column description is required in the representation,
            fieldWithVal if value is required in the representation,
            fieldWithCasts if cast and column renaming is required
        \return a vector of string representations
    */
    virtual std::vector<std::string> fields(const std::vector<column_t> &cols,
                                            FieldAction action = &CellRepresentation::field) const;

public:
    /*!
        \brief Convert a hash of a type to a string representing SQL type
        \param hash hash of a C++ type
        \return a string that represents an SQL type
    */
    static std::string type(std::size_t hash);

    /*!
        \brief Creates a description of a Cell (parameters and constraints)
        \param cell a Cell object
        \return a string that represents the description
    */
    virtual std::string description(const Cell &cell) const;

    // Helper functions that might be useful for child classes

    /*!
        \brief Converts a vector of columns to a vector of their string representations.
        The representations contain only column names.
        \param cols a vector of column_t objects
        \return a vector of string representations
    */
    std::vector<std::string> fieldsNames(const std::vector<column_t> &cols) const;

    /*!
        \brief Converts a vector of columns to a vector of their string representations.
        The representations contain only column alieases (unqualified names).
        \param cols a vector of column_t objects
        \return a vector of string representations
    */
    std::vector<std::string> fieldsShortNames(const std::vector<column_t> &cols) const;

    /*!
        \brief Converts a vector of columns to a vector of their string representations.
        The representations contain only column alieases (unqualified names).
        \param cells a vector of Cell
        \return a vector of string representations
    */
    std::vector<std::string> fieldsShortNames(const std::vector<Cell> &cells) const;

    /*!
        \brief Converts a vector of columns to a vector of their string representations.
        The representations contain column names and their description (e.g. types, default values).
        \param cols a vector of column_t objects
        \return a vector of string representations
    */
    std::vector<std::string> fieldsWithDescr(const std::vector<column_t> &cols) const;

    /*!
        \brief Converts a vector of columns to a vector of their string representations.
        The representations contain column names with casts to their types.
        \param cols a vector of column_t objects
        \return a vector of string representations
    */
    std::vector<std::string> fieldsWithCasts(const std::vector<column_t> &cols) const;

    /*!
        \brief Converts a type name to the type required to cast to it.
        In most cases it will return its argument, but sometimes conversion is required
        (e.g. MySQL requires "col AS SIGNED" when "col" type is INTEGER)
        \param cols a vector of column_t objects
        \return a vector of string representations
    */
    virtual std::string typeToCastType(const std::string &typeName) const;

    /*!
        \brief Converts a vector of column_t to a vector of string representations
        \param cols a vector of column_t objects
        \return a vector of string representations
    */
    virtual std::vector<SqlValue> values(const std::vector<column_t> &cols) const;

    /*!
        \brief Converts a vector of column_t to a vector of string representations
        \param cells a vector of Cell
        \return a vector of string representations
    */
    virtual std::vector<SqlValue> values(const std::vector<Cell> &cells) const;

    /*!
        \brief Converts a cell to a column_t object
        \param cell a cell
        \return a column_t object
    */
    virtual column_t column(const Cell &cell) const;

    /*!
        \brief Converts a vector of cells to a vector of column_t objects
        \param cells vector of Cell objects
        \return a vector of column_t
    */
    virtual std::vector<column_t> columns(const std::vector<Cell> &cells) const;

    /*!
        \brief Retrieves column from a query in a form of a vector of column_t object
        \param query SqlQuery object
        \return a vector of column_t
    */
    virtual std::vector<column_t> columns(const class SqlQuery &query) const;

    /*!
        \brief Fills missing .defval in dstCols which does not have corresponding
        column in srcCols.
        \param srcCols vector of column_t which values in dstCols should not be changed
        \param dstCols vector of column_t which .defval needs to be filled
        \param defval the value that will be filled in appropriate columns in dstCols
    */
    static void fillMissingDefaults(const std::vector<column_t> &srcCols, std::vector<column_t> &dstCols,
                                    const SqlValue &defval = SqlValue::Null());
};

} // namespace db
} // namespace softeq

#endif
