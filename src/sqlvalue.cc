#include "sqlvalue.hh"

namespace softeq
{
namespace db
{
SqlValue SqlValue::Null()
{
    SqlValue ret(0);
    ret._subtype = SqlValue::Subtype::Null;

    return ret;
}

std::string SqlValue::toString() const
{
    switch (_subtype)
    {
    case Subtype::String:
        return _value;

    case Subtype::Integer:
        return std::to_string(_intvalue);

    case Subtype::Null:
        return "NULL";

    default:
        return _value;
    }
}
} // namespace db
} // namespace softeq
