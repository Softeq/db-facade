#ifndef SOFTEQ_DBFACADE_TYPESERIALIZERS_H_
#define SOFTEQ_DBFACADE_TYPESERIALIZERS_H_

#include <cstdint>
#include <memory>

#include "typeconverter.hh"
#include "typehint.hh"

namespace softeq
{
namespace db
{

namespace type_serializers
{

/**
 * \brief Serializer type template declaration
 * 
 */
template <typename T, typename = void>
struct serialize;

/**
 * \brief Serializer definition for non nullable unique_ptr field types
 */
template <typename T>
struct serialize<std::unique_ptr<T>>
{
    static TypeHint getTypeHint()
    {
        return serialize<T>::getTypeHint();
    }

    static SqlValue from(const std::unique_ptr<T>& from)
    {
        return serialize<T>::from(*from);
    }

    static std::unique_ptr<T> to(const char* from)
    {
        return std::unique_ptr<T>(new T(serialize<T>::to(from)));
    }
};

/**
 * \brief Serializer class definition for Integral types
 */
template <typename Integral>
struct serialize<Integral, typename std::enable_if<std::is_integral<Integral>::value>::type>
{
    static TypeHint getTypeHint()
    {
        return TypeHint(TypeHint::InnerType::Integer, sizeof(Integral));
    }

    static SqlValue from(const Integral& from)
    {
        return SqlValue(static_cast<int64_t>(from));
    }

    static Integral to(const char* from)
    {
        Integral num;
        std::stringstream ss(from);
        ss >> num;
        return num;
    }
};

/**
 * \brief Serializer class definition for types convertavle to/from string
 * 
 * \tparam String 
 */
template <typename String>
struct serialize <String, typename std::enable_if<std::is_convertible<String, std::string>::value>::type>
{
    static TypeHint getTypeHint()
    {
        return TypeHint(TypeHint::InnerType::String);
    }

    static SqlValue from(const String &from)
    {
        return SqlValue(std::string(from));
    }

    static String to(const char* from)
    {
        return String(std::string(from)); 
    }
};

} // namespace type_serializers
} // namespace db
} // namespace softeq

#endif //SOFTEQ_DBFACADE_TYPESERIALIZERS_H_
