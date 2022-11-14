#define _POSIX_C_SOURCE 200112L // for gmtime_r

#include <stdexcept>

#include "columndatetime.hh"

namespace softeq
{
namespace db
{
namespace columntypes
{
namespace impl
{
SqlValue epochTimeToString(const std::time_t &time)
{
    std::tm tmstruct;
    char buf[sizeof("'YYYY-MM-DD HH:MM:SS.SSS'")] = {0};

    gmtime_r(&time, &tmstruct);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %X.000", &tmstruct);

    return SqlValue(std::string(buf));
};

std::time_t stringToEpochTime(const char *timestr)
{
    constexpr int numbersInDate = 6;
    std::tm tmstruct = {};

    int scannedNums = std::sscanf(timestr, "%d-%d-%d %d:%d:%d", &tmstruct.tm_year, &tmstruct.tm_mon, &tmstruct.tm_mday,
                                  &tmstruct.tm_hour, &tmstruct.tm_min, &tmstruct.tm_sec);

    if (scannedNums == numbersInDate)
    {
        tmstruct.tm_year -= 1900;
        tmstruct.tm_mon -= 1;

        return std::mktime(&tmstruct) - timezone;
    }
    throw std::invalid_argument("Can't parse time " + std::string{timestr});
}
} // namespace impl

void DateTime(TypeConverter<std::time_t> &converter)
{
    converter = TypeConverter<std::time_t>{false, toDatabaseType(TypeHint(TypeHint::InnerType::DateTime)),
                                           [](const std::time_t &value) { return impl::epochTimeToString(value); },
                                           [](const char *value) { return impl::stringToEpochTime(value); }};
}

} // namespace columntypes
} // namespace db
} // namespace softeq
