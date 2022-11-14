#ifndef SOFTEQ_DBFACADE_TYPEHINT_H_
#define SOFTEQ_DBFACADE_TYPEHINT_H_

#include <cstdint>

namespace softeq
{
namespace db
{

/**
 * \brief Hint class used to define database type hint information
 */
struct TypeHint
{
    /**
     * \brief Supposed type of the database field
     */
    enum class InnerType
    {
        Integer,  /// field is likely integer
        Binary,   /// field is likely binary blob
        String,   /// field is likely string
        DateTime, /// field is datetime
    };

    /**
     * \brief Construct a new Type Hint object
     * 
     * \param hintType  Hint type
     */
    explicit TypeHint(const InnerType &hintType)
        : hintType(hintType)
        , hintSize(0)
    {
    }

    /**
     * \brief Construct a new Type Hint object
     * 
     * \param hintType  Hint type
     * \param hintSize  Hint type size in bytes
     */
    TypeHint(const InnerType &hintType, size_t hintSize)
        : hintType(hintType)
        , hintSize(hintSize)
    {
    }

    InnerType hintType;  /// Hint type
    size_t hintSize;     /// Hint size in bytes
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_TYPEHINT_H_
