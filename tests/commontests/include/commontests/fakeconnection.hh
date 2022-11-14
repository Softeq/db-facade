#pragma once

#include <sstream>
#include <dbfacade/facade.hh>
#include <dbfacade/sqlquery.hh>

namespace softeq
{
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v)
{
    if (!v.empty())
    {
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out.seekp(-2, std::ios_base::cur);
    }
    else
    {
        out << "*";
    }

    out.flush();
    return out;
}

class FakeConnection : public db::Connection
{
public:
    std::string lastQuery() const
    {
        return _lastQuery;
    }

    bool perform(const db::SqlQuery &query, db::Connection::parseFunc fn) override
    {
        std::stringstream ss;
        ss << query;
        _lastQuery = ss.str();
        return true;
    }

private:
    std::string _lastQuery;
};
} // namespace softeq

