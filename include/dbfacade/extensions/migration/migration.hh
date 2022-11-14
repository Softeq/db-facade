#ifndef SOFTEQ_MIGRATION_DATABASE_HH
#define SOFTEQ_MIGRATION_DATABASE_HH
#include <algorithm>
#include <list>
#include <dbfacade/facade.hh>
#include <dbfacade/drop.hh>
#include <dbfacade/alter.hh>
#include <dbfacade/createtable.hh>
#include <common/migration/migration_manager.hh>

namespace softeq
{
namespace db
{
namespace migration
{
/* Task that modifies table structure in database
 * It adds new columns, that are specified in T_NEW class
 * and removes column that are specified in T_OLD clas but don't exist in T_NEW class
 */
template <typename T_OLD, typename T_NEW>
class AlterTableTask : public softeq::common::migration::Task
{
public:
    /*!
     * \brief Constructor of AlterTable class
     * \param db[in] database object to use during update
     */
    AlterTableTask(const softeq::common::migration::Version &from, db::Facade &db,
                   const std::vector<std::pair<db::CellMaker, db::CellMaker>> &renameCells = {})
        : softeq::common::migration::Task(from, std::bind(&AlterTableTask::update,
                                          this, std::ref(db), renameCells))
    {
        setDescription("change columns of the table: " + db::buildTableScheme<T_OLD>().name());
    }

private:
    bool update(db::Facade &db, const std::vector<std::pair<db::CellMaker, db::CellMaker>> &renameCells = {})
    {
        try
        {
            auto query = db::query::alterScheme<T_OLD, T_NEW>();
            for (const auto &renamePair : renameCells)
            {
                query.renamingCell(renamePair.first, renamePair.second);
            }
            db.execute(query);
        }
        catch (db::SqlException &)
        {
            // probably throwing an exeption would be a better idea
            return false;
        }
        return true;
    }
};

/* Task that creates a new table in database */
template <typename T>
class CreateTableTask : public softeq::common::migration::Task
{
public:
    /*!
     * \brief Constructor of CreateTableTask class
     * \param db[in] database object to use during update
     */
    CreateTableTask(const softeq::common::migration::Version &from, db::Facade &db)
        : softeq::common::migration::Task(from, std::bind(&CreateTableTask::update, this, std::ref(db)))
    {
        setDescription("create table: " + db::buildTableScheme<T>().name());
    }

private:
    bool update(db::Facade &db)
    {
        try
        {
            db.execute(db::query::createTable<T>());
        }
        catch (db::SqlException &)
        {
            // probably throwing an exeption would be a better idea
            return false;
        }
        return true;
    }
};

/* Task that deletes table from database */
template <typename T>
class DeleteTableTask : public softeq::common::migration::Task
{
public:
    /*!
     * \brief Constructor of DeleteTableTask class
     * \param db[in] database object to use during update
     */
    DeleteTableTask(const softeq::common::migration::Version &from, db::Facade &db)
        : softeq::common::migration::Task(from, std::bind(&DeleteTableTask::update, this, std::ref(db)))
    {
        setDescription("delete table: " + db::buildTableScheme<T>().name());
    }

private:
    bool update(db::Facade &db)
    {
        try
        {
            db.execute(db::query::drop<T>());
        }
        catch (db::SqlException &)
        {
            // probably throwing an exeption would be a better idea
            return false;
        }
        return true;
    }
};

} // namespace migration
} // namespace db
} // namespace softeq
#endif
