#ifndef SOFTEQ_DBFACADE_COLUMNDATETIME_H_
#define SOFTEQ_DBFACADE_COLUMNDATETIME_H_

#include <chrono>
#include <memory>

#include "columntypes.hh"
#include "typeconverter.hh"

namespace softeq
{
namespace db
{
namespace columntypes
{
namespace impl
{
/*!
    \brief Converts std::time_t time to ISO 8601 string representation
    \param time time_t time (seconds since 1970-01-01 00:00:00)
    \return string representing time in ISO 8601 format
*/
SqlValue epochTimeToString(const std::time_t &time);

/*!
    \brief Converts time represented as ISO 8601 string to time_t
    \param timestr time represented as ISO 8601 string
    \return time_t time (seconds since 1970-01-01 00:00:00)
*/
std::time_t stringToEpochTime(const char *timestr);
} // namespace impl

/**
 * \brief DateTime type converter
 *
 * \param converter
 */
void DateTime(TypeConverter<std::time_t> &converter);

} // namespace columntypes
} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_COLUMNDATETIME_H_