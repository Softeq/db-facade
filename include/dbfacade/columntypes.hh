#ifndef SOFTEQ_DBFACADE_FIELDTYPES_H_
#define SOFTEQ_DBFACADE_FIELDTYPES_H_

#include <memory>

#include "typeconverter.hh"
#include "typehint.hh"
#include "typeserializers.hh"

namespace softeq
{
namespace db
{
namespace columntypes
{
/**
 * \brief Selects database type by type hint information
 *
 * \param hint     Hint information
 * \return size_t  typeHash of the database type
 */
size_t toDatabaseType(const TypeHint &hint);

/**
 * \brief Standard column type converter helper
 */
template <typename T>
void Standard(TypeConverter<T> &converter)
{
    converter = {
        .isNullable = false,
        .typeHash = toDatabaseType(::softeq::db::type_serializers::serialize<T>::getTypeHint()),
        .from = [](const T &value)  { return ::softeq::db::type_serializers::serialize<T>::from(value); },
        .to = [](const char *value) { return ::softeq::db::type_serializers::serialize<T>::to(value); }};
}

/**
 * \brief Nullable column type helper
 */
template <typename Optional>
void Nullable(TypeConverter<std::unique_ptr<Optional>> &converter)
{
    // clang-format off
    converter = 
        {
         .isNullable = true,
         .typeHash = toDatabaseType(::softeq::db::type_serializers::serialize<Optional>::getTypeHint()),
         .from = [](const std::unique_ptr<Optional> &value)
         {
             if (value)
             {
                 return ::softeq::db::type_serializers::serialize<Optional>::from(*value);
             }

             return SqlValue::Null();
         },
         .to =
             [](const char *value)
         {
             if (value == nullptr)
             {
                 return std::unique_ptr<Optional>();
             }
             return std::unique_ptr<Optional>(
                 new Optional(::softeq::db::type_serializers::serialize<Optional>::to(value)));
         }};
    // clang-format on
}

} // namespace columntypes
} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_FIELDTYPES_H_
