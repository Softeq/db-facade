#ifndef SOFTEQ_DBFACADE_SCHEME_H_
#define SOFTEQ_DBFACADE_SCHEME_H_

#include "cell.hh"
#include "base_constraint.hh"

namespace softeq
{
namespace db
{
/*!
    \brief this method should be implemented for all structures that
    represent a table in a database.
 */
template <typename T>
const class TableScheme buildTableScheme();

/*!
    \brief A class which represents a correspondence between C++ structure and a
    database table. It's used to emulate reflection in C++.
 */
class TableScheme
{
public:
    using Cells = std::vector<Cell>;

    // TODO: try to remove this constructor when it's no longer needed and add 'const' to _cells
    TableScheme() = default;

    /*!
        \brief Construct a scheme
        \param name name of table
        \param cells cells of a table
     */
    TableScheme(const std::string &name, std::initializer_list<Cell> cells);
    /*!
        \brief Construct a scheme
        \param name name of table
        \param cells cells of a table
        \param constraints list of constraints applied to the table
     */
    TableScheme(const std::string &name, std::initializer_list<Cell> cells,
                std::initializer_list<std::shared_ptr<constraints::BaseConstraint>> constraints);
    /*!
        \brief Construct a scheme
        \param name name of table
        \param cells cells of a table (std::vcetor)
     */
    TableScheme(const std::string &name, Cells &&cells);

    // Accessors

    Cells cells() const
    {
        return _cells;
    }

    constraints::Constraints constraints() const
    {
        return _constraints;
    }

    /*!
        \brief Gets a cell in a scheme by its offset
        \param offset offset of cell
        \return the cell corresponding to the offset
        \throw std::logic::exception if there is no such cell
     */
    Cell cell(std::ptrdiff_t offset) const;

    /*!
        \brief find a cell in a scheme by its name
        \param name name of cell
        \return a pair: the first element is a cell and
        the second one is true if the element has been found
     */
    std::pair<Cell, bool> findCell(const std::string &name) const;

    /*!
     * \brief get name of the table
     * \return name of the table
     */
    const std::string &name() const
    {
        return _name;
    }

    // Types and methods responsible for schemes' conversion

    /*!
        \brief Enumeration that reprents a type of change
    */
    enum DiffActionType
    {
        NO_OP,
        RENAME_TABLE,
        ADD_COLUMN,
        DROP_COLUMN, // sqlite does not support DROP COLUMN
        RENAME_COLUMN
    };

    /*!
        \brief Struct that represents a type of action and action parameters
     */
    struct DiffActionItem
    {
        DiffActionType type;

        // maybe we can use a union, but we will have to manage destructors
        Cell cell;                        // add column, potential remove or modify column
        std::string table;                // table to rename
        std::pair<Cell, Cell> renameCell; // rename column

        // method-constructors that create actions

        static DiffActionItem noop();
        static DiffActionItem renameTable(const std::string &newName);
        static DiffActionItem addColumn(const Cell &cell);
        static DiffActionItem dropColumn(const Cell &cell);
        static DiffActionItem renameColumn(const Cell &from, const Cell &to);
    };

    using DiffActionItems = std::vector<DiffActionItem>;

    /*!
        \brief Generates conversion steps to convert this scheme to toScheme
        \param toScheme the scheme to convert to
        \return vector conversion steps
    */
    DiffActionItems generateConversionSteps(const TableScheme &toScheme) const;

    /*!
        \brief Generates conversion steps to convert FromStruct scheme to ToStruct
        \tparam FromStruct the struct to convert from
        \tparam ToStruct the struct to convert to
        \return vector conversion steps
    */
    template <typename FromStruct, typename ToStruct>
    static DiffActionItems generateConversionSteps()
    {
        return buildTableScheme<FromStruct>().generateConversionSteps(buildTableScheme<ToStruct>());
    }

    static void renameColumn(DiffActionItems &items, const Cell &from, const Cell &to);

private:
    std::string _name;
    std::vector<Cell> _cells;
    constraints::Constraints _constraints;
};

/*!
    \brief A class that creates a full qualified cell with all fields filled.
 */
class CellMaker
{
    Cell _cell;

public:
    /*!
        \brief Construct an oject that fills in the cell.
        \param member struct member
     */
    template <typename Struct, typename T>
    CellMaker(T Struct::*member)
    {
        auto scheme = buildTableScheme<Struct>();
        _cell = scheme.cell(Cell::fieldOffset<Struct, T>(member));
        _cell.setTable(scheme.name());
    }

    /*!
        \brief Returns the cell with all fields filled.
        \return the cell
     */
    const Cell &operator()() const
    {
        return _cell;
    }
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SCHEME_H_
