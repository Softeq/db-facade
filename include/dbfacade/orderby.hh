#ifndef SOFTEQ_DBFACADE_ORDERBY_H_
#define SOFTEQ_DBFACADE_ORDERBY_H_

#include <ostream>
#include "tablescheme.hh"

namespace softeq
{
namespace db
{
struct OrderBy
{
    Cell cell;
    enum OrderType
    {
        ASC,
        DESC
    } order;

    OrderBy(const CellMaker &cell, OrderType order)
        : cell(cell())
        , order(order)
    {
    }

    template <typename Struct, typename T>
    OrderBy(T Struct::*member)
        : cell(CellMaker(member)())
        , order(ASC)
    {
    }

    OrderBy(const OrderBy &) = default;
    OrderBy(OrderBy &&) = default;
};

inline std::ostream &operator<<(std::ostream &out, const OrderBy &orderby)
{
    return out << orderby.cell.name() << ' ' << (orderby.order == OrderBy::ASC ? "ASC" : "DESC");
}

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_ORDERBY_H_
