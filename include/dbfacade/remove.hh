#ifndef SOFTEQ_DBFACADE_REMOVE_H_
#define SOFTEQ_DBFACADE_REMOVE_H_

#include "sqlquery.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class for deleting data from the specified table
*/
class RemoveQuery : public SqlConditionalQuery<RemoveQuery>
{
public:
    explicit RemoveQuery(const TableScheme &scheme);
};

namespace query
{
/*!
    \brief Forms a DELETE request for the selected table
    \tparam <Struct> containing the schema of the database table
*/
template <typename Struct>
RemoveQuery remove()
{
    return RemoveQuery(buildTableScheme<Struct>());
}
} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_REMOVE_H_
