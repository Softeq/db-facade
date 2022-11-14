#ifndef SOFTEQ_DBFACADE_LIMIT_H_
#define SOFTEQ_DBFACADE_LIMIT_H_

#include <cstdint>
#include <limits>

namespace softeq
{
namespace db
{
/*!
    \brief The struct represents the limits of result of a select query, namely starting offset
    and the max number of rows to fetch.
*/
struct ResultLimit
{
private:
    static constexpr std::uint64_t infinity = std::numeric_limits<std::uint64_t>::max();

public:
    /*!
        \brief The offset at which we need to start fetching rows.
        E.g. if 1 is specified, the first row will be skipped
    */
    std::uint64_t rowsOffset = 0;

    /*!
        \brief The max numer of rows that need to be in the result
    */
    std::uint64_t rowsLimit = infinity;

    /*!
        \brief Checks if the limit is defined (i.e. it actually limits the result)
        \return true if defined, otherwise false
    */
    bool defined() const;

    /*!
        \brief Checks if the limit (rowLimit field) is finite
        \return true if finite, otherwise false
    */
    bool finite() const;
};

} // namespace db
} // namespace softeq

#endif
