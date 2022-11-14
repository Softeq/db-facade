#include "join.hh"

namespace softeq
{
namespace db
{
Join::Join(const std::string &name, const Condition &condition)
    : _name(name)
    , _condition(condition)
{
}

const std::string &Join::name() const
{
    return _name;
}

const Condition &Join::condition() const
{
    return _condition;
}

std::vector<Token> Join::tokens() const
{
    std::vector<Token> tokens;
    tokens << name() << " ON " << condition().tokens();
    return tokens;
}

} // namespace db
} // namespace softeq
