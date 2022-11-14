#ifndef SOFTEQ_DBFACADE_JOIN_H_
#define SOFTEQ_DBFACADE_JOIN_H_

#include <string>

#include "condition.hh"
#include "token.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class which represents a JOIN clause.
    It's supposed to be used through facade.
*/
class Join
{
public:
    /*!
        \brief Construct a JOIN clause using
    */
    Join(const std::string &name, const Condition &condition);

    const std::string &name() const;
    const Condition &condition() const;

    std::vector<Token> tokens() const;    

private:
    std::string _name;
    Condition _condition;
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_JOIN_H_
