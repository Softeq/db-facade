#ifndef SOFTEQ_DBFACADE_DROP_H_
#define SOFTEQ_DBFACADE_DROP_H_

#include "sqlquery.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class for dropping the entire specified table
*/
class DropQuery : public SerializableSqlQuery<DropQuery>
{
public:
    explicit DropQuery(const TableScheme &scheme);
};

namespace query
{
/*!
    \brief Method forms an DROP query that drops a table
    \tparam <Struct> containing the schema of the database table
*/
template <typename Struct>
DropQuery drop()
{
    return DropQuery(buildTableScheme<Struct>());
}
} // namespace query

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_DROP_H_
