
#include <chrono>

#include "columntypes.hh"

namespace softeq
{
namespace db
{
namespace columntypes
{

size_t toDatabaseType(const TypeHint &from)
{
    size_t hashCode{};
    switch (from.hintType)
    {
    case TypeHint::InnerType::Binary:
        hashCode = typeid(std::string).hash_code();
        break;

    case TypeHint::InnerType::Integer:
        hashCode = typeid(int).hash_code();
        break;

    case TypeHint::InnerType::String:
        hashCode = typeid(std::string).hash_code();
        break;

    case TypeHint::InnerType::DateTime:
        hashCode = typeid(std::chrono::time_point<std::chrono::system_clock>).hash_code();
        break;
    }
    return hashCode;
}

} // namespace columntypes
} // namespace db
} // namespace softeq
