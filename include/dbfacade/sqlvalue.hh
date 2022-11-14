#ifndef SOFTEQ_DBFACADE_SQLVALUE_H_
#define SOFTEQ_DBFACADE_SQLVALUE_H_

#include <string>

#include "typehint.hh"

namespace softeq
{
namespace db
{
class SqlValue
{
public:
    enum class Subtype
    {
        Null,
        String,
        Integer,
        DateTime,
        Blob,
        Empty,
    };

    explicit SqlValue()
        : _subtype(Subtype::Empty)
    {
    }

    explicit SqlValue(std::string &&from)
        : _value(std::move(from))
    {
    }

    explicit SqlValue(int64_t from)
        : _intvalue(from)
        , _subtype(Subtype::Integer)
    {
    }

    static SqlValue Null();

    std::string &strValue()
    {
        return _value;
    }

    const std::string &strValue() const
    {
        return _value;
    }

    int64_t &intValue()
    {
        return _intvalue;
    }

    int64_t intValue() const
    {
        return _intvalue;
    }

    Subtype type() const
    {
        return _subtype;
    }

    std::string toString() const;

private:
    std::string _value;
    int64_t _intvalue = 0;
    Subtype _subtype = Subtype::String;
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_SQLVALUE_H_