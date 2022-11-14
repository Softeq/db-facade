#include "resultlimit.hh"

namespace softeq
{
namespace db
{
bool ResultLimit::defined() const
{
    return rowsOffset > 0 || finite();
}

bool ResultLimit::finite() const
{
    return rowsLimit != infinity;
}

} // namespace db
} // namespace softeq
