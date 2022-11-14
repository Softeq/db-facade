#ifndef SOFTEQ_DBFACADE_ALTER_H_
#define SOFTEQ_DBFACADE_ALTER_H_

#include "sqlquery.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class used to add, delete, or modify columns in an existing table.
*/
class AlterQuery : public SerializableSqlQuery<AlterQuery>
{
public:
    explicit AlterQuery(const softeq::db::TableScheme &scheme);

    /*!
        \brief The method allows a user to specify which columns need to be renamed
        instead of being dropped and added.
        \param oldCell cell that which needs to be renamed
        \param newCell cell which oldCell must be renamed to
        \return this AlterQuery object with modifications
    */
    AlterQuery &renamingCell(const CellMaker &oldCell, const CellMaker &newCell);

    /*!
        \brief Gets scheme conversion steps
        \return conversion steps
    */
    const TableScheme::DiffActionItems &alters() const;

    /*!
        \brief The method specifies that difference between two schemes must be calculated automatically.
        Note that there is no way to automatically find out that a column must be renamed, such alterations
        must be explicitly specified using renamingCell method.
        \return this AlterQuery object with added ALTER clause
    */
    template <typename OldStruct, typename NewStruct>
    AlterQuery &autoAlter()
    {
        _alters = TableScheme::generateConversionSteps<OldStruct, NewStruct>();
        return *this;
    }

private:
    TableScheme::DiffActionItems _alters;
};

namespace query
{
/*!
    \brief Method forms an ALTER TABLE query that generates queries for
    altering a table (add columns, rename table)
    \tparam <OldStruct> the schema of the previous table
    \tparam <NewStruct> the schema of the newer table
*/
template <typename OldStruct, typename NewStruct>
AlterQuery alterScheme()
{
    return AlterQuery(buildTableScheme<OldStruct>()).template autoAlter<OldStruct, NewStruct>();
}
} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_ALTER_H_
