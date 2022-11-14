#ifndef SOFTEQ_DBFACADE_BASE_CONSTRAINT_H_
#define SOFTEQ_DBFACADE_BASE_CONSTRAINT_H_

#include <vector>
#include <memory>

namespace softeq
{
namespace db
{
class SqlQueryStringBuilder;
class TableScheme;
namespace constraints
{
class BaseConstraint
{
public:
    virtual std::string toString(const class SqlQueryStringBuilder &builder, const class TableScheme &scheme) const = 0;

    virtual ~BaseConstraint()
    {
    }
};

// TODO: add named title for the Constraints section
using Constraints = std::vector<std::shared_ptr<BaseConstraint>>;

template <class T, typename... A>
std::shared_ptr<BaseConstraint> makeConstraint(A &&... args)
{
    return std::make_shared<T>(std::forward<A>(args)...);
}

template <class A, class B>
std::pair<A, B> addCascade(A a, B b)
{
    return std::make_pair(a, b);
}

} // namespace constraints
} // namespace db
} // namespace softeq

#endif
