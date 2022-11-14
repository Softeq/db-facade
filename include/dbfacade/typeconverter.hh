#ifndef SOFTEQ_DBFACADE_TYPECONVERTER_H_
#define SOFTEQ_DBFACADE_TYPECONVERTER_H_

#include <functional>
#include <string>

#include "sqlvalue.hh"

namespace softeq
{
namespace db
{
/*!
    \brief A structure for a custom type converter. A user should define all members and pass it to Cell constructor.
    'from' function should return an object of internal type (string) or it may return SqlValue::Null() if isNullabel == true.
    'to' function should expect that its parameter can be nullptr if the field is nullable (isNullable == true).
*/
template <typename T>
struct TypeConverter
{
    bool isNullable;                            //! This value will be stored as NULLABLE
    size_t typeHash;                            //! Database serialization type (INTEGER, TEXT, etc...))
    std::function<SqlValue(const T &)> from; //! creates a object of internal type (string) from type T
    std::function<T(const char *)> to;          //! converts an internal type (string) to type T
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_TYPECONVERTER_H_
