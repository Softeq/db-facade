#include "version.hh"

#define STR(s) #s
#define STRVAL(v) STR(v)

namespace softeq::db
{
std::string getVersion()
{
    return STRVAL(PROJECT_VERSION_GENERATION) "." STRVAL(PROJECT_VERSION_MAJOR) "." STRVAL(PROJECT_VERSION_MINOR);
}

std::string getComponents()
{
    return STRVAL(COMPONENTS_LIST);
}

} // namespace softeq::db
