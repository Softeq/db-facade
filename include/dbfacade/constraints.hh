#ifndef SOFTEQ_DBFACADE_CONSTRAINTS_H_
#define SOFTEQ_DBFACADE_CONSTRAINTS_H_

#include "tablescheme.hh"
#include "base_constraint.hh"
#include "condition.hh"

namespace softeq
{
namespace db
{
class SqlQueryStringBuilder;
class Condition;
class TableScheme;
namespace constraints
{
enum CascadeAction
{
    NoAction,
    Restrict,
    SetNull,
    SetDefault,
    Cascade,
};

enum CascadeTrigger
{
    OnUpdate,
    OnDelete
};

/*!
\brief Class for keeping Foreign key constraint. It works for 1-1 relations.
*/
class ForeignKeyConstraint : public BaseConstraint
{
public:
    template <typename StructOwn, typename O, typename StructForeign, typename F, typename... R>
    ForeignKeyConstraint(O StructOwn::*ownMember, F StructForeign::*foreignMember, R &&... triggers)
        : _cell(ownMember)
        , _foreignCell(CellMaker(foreignMember))
    {
        pushCascadeAction(std::forward<R>(triggers)...);
    }

    virtual std::string toString(const class SqlQueryStringBuilder &builder,
                                 const class TableScheme &scheme) const override;
    const Cell &cell() const;
    const Cell &foreignCell() const;
    const std::vector<std::pair<CascadeTrigger, CascadeAction>> &triggers() const;

private:
    const Cell _cell;
    const CellMaker _foreignCell;
    std::vector<std::pair<CascadeTrigger, CascadeAction>> _triggers;

    template <typename... R>
    void pushCascadeAction(std::pair<CascadeTrigger, CascadeAction> action, R &&... rest)
    {
        _triggers.push_back(action);
        pushCascadeAction(std::forward<R>(rest)...);
    }

    void pushCascadeAction()
    {
    }
};

std::ostream &operator<<(std::ostream &os, const CascadeTrigger &trigger);
std::ostream &operator<<(std::ostream &os, const CascadeAction &action);

// TODO: Rework flags as constrints, add DEFAULT inline constraint
// TODO: Add primary key handling as a separate constraint derived from base_constraint
// TODO: Add check if parent referenced cell is a primary key
// TODO: Add check constraint
// TODO: Make cell update query working if: we need to update primary key, we want to update only one field

} // namespace constraints
} // namespace db
} // namespace softeq

#endif
