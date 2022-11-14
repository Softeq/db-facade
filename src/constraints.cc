#include "constraints.hh"
#include "sqlquerybuilder.hh"
#include "tablescheme.hh"

namespace softeq
{
namespace db
{
namespace constraints
{
const Cell &ForeignKeyConstraint::cell() const
{
    return _cell;
}

const Cell &ForeignKeyConstraint::foreignCell() const
{
    return _foreignCell();
}

const std::vector<std::pair<CascadeTrigger, CascadeAction>> &ForeignKeyConstraint::triggers() const
{
    return _triggers;
}

std::string ForeignKeyConstraint::toString(const class SqlQueryStringBuilder &builder,
                                           const class TableScheme &scheme) const // TODO: put override
{
    return builder.toString(*this, scheme);
}

std::ostream &operator<<(std::ostream &os, const CascadeTrigger &trigger)
{
    switch (trigger)
    {
    case constraints::CascadeTrigger::OnDelete:
        os << "ON DELETE";
        break;
    case constraints::CascadeTrigger::OnUpdate:
        os << "ON UPDATE";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const CascadeAction &action)
{
    switch (action)
    {
    case constraints::CascadeAction::NoAction:
        os << "NO ACTION";
        break;
    case constraints::CascadeAction::Cascade:
        os << "CASCADE";
        break;
    case constraints::CascadeAction::Restrict:
        os << "RESTRICT";
        break;
    case constraints::CascadeAction::SetDefault:
        os << "SET DEFAULT";
        break;
    case constraints::CascadeAction::SetNull:
        os << "SET NULL";
        break;
    }
    return os;
}

} // namespace constraints
} // namespace db
} // namespace softeq
