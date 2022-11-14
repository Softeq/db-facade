#ifndef SOFTEQ_DBFACADE_CELL_H_
#define SOFTEQ_DBFACADE_CELL_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "columntypes.hh"
#include "sqlvalue.hh"
#include "typeconverter.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class that represents a column in a database and it's relation to C++ struct
*/
class Cell
{
public:
    enum Flags : std::uint32_t
    {
        NONE = 0,           // - No flags
        UNIQUE = 2,         // - Ensures that all values in a column are different
        PRIMARY_KEY = 4,    // - A combination of a NOT NULL and UNIQUE. Uniquely identifies each row in a table
        CHECK = 16,         // - Ensures that all values in a column satisfies a specific condition
        DEFAULT = 32,       // - Sets a default value for a column when no value is specified
        AUTOINCREMENT = 64, // - Auto-increment allows a unique number to be generated automatically when a new
                            // record is inserted into a table.
        CUSTOM = 256,       // - data is in config
    };

    explicit Cell(std::nullptr_t)
    {
    }

    Cell()
        : Cell(nullptr)
    {
    }

    /*!
        \brief Construct a cell by a struct member.
        A cell created this way will not have name, table or flags.

        \param member member of a struct, e.g. &table::field.
        \note This method is used only in CellMaker to get field offset
     */
    template <typename Struct, typename T>
    explicit Cell(T Struct::*member)
        : _offset(fieldOffset(member))
    {
    }

    /*!
        \brief Construct Cell for structure's field

        \tparam Struct    Structure type type parameter
        \tparam T         Structure field type type parameter
        \param member     Construct Cell for this structure member
        \param name       Name of the cell/column
        \param typeTrait  Cell Trait
        \param flags      Additional Cell Flags
    */
    template <typename Struct, typename T>
    Cell(T Struct::*member, const std::string &name, void (*typeTrait)(TypeConverter<T> &),
         std::uint32_t flags = Flags::NONE)
    {
        TypeConverter<T> converter;
        typeTrait(converter);

        _typeHash = converter.typeHash;
        _type = std::make_shared<Holder<Struct>>(member, converter);

        _offset = fieldOffset(member);
        _name = name;
        _flags = flags;

        _isNullable = converter.isNullable;
    }

    /*!
        \brief Construct Cell for structure's field with an option to specify default value

        \tparam Struct    Structure type type parameter
        \tparam T         Structure field type type parameter
        \param member     Construct Cell for this structure member
        \param name       Name of the cell/column
        \param typeTrait  Cell Trait
        \param flags      Additional Cell Flags
        \param config     Additional configuration like default value if value is not present in cell/database column
    */
    template <typename Struct, typename T, typename ConfigT,
              typename std::enable_if<std::is_constructible<T, ConfigT>::value>::type * = nullptr>
    Cell(T Struct::*member, const std::string &name, void (*typeTrait)(TypeConverter<T> &), std::uint32_t flags,
         const ConfigT &config)
        : Cell(member, name, typeTrait, flags)
    {
        TypeConverter<T> converter;
        typeTrait(converter);

        _config = converter.from(config);
    }

    /*!
        \brief Construct a cell with name, flags and C++ struct member.

        It's main purpose is to create a cell in buildTableScheme method. A cell
        created this way will not have will not contain information about table
        name.

        \param member struct member
        \param name name of the cell/column
        \param flags flag set made of Cell::Flags enum
    */
    template <typename Struct, typename T>
    Cell(T Struct::*member, const std::string &name, std::uint32_t flags = Flags::NONE)
        : Cell(member, name, columntypes::Standard, flags)
    {
    }

    /*!
        \brief Construct a cell with name, flags and C++ struct member.

        It's main purpose is to create a cell in buildTableScheme method. A cell
        created this way will not have will not contain information about table
        name.

        \param member struct member
        \param name name of the cell/column
        \param flags flag set made of Cell::Flags enum
        \param config additional configuration
    */
    template <typename Struct, typename T, typename ConfigT,
              typename std::enable_if<std::is_constructible<T, ConfigT>::value>::type * = nullptr>
    Cell(T Struct::*member, const std::string &name, std::uint32_t flags, const ConfigT &config)
        : Cell(member, name, columntypes::Standard, flags, config)
    {
    }

    /*!
        \brief Returns either a name of the cell or a fully qualified name (table.column)
     */
    std::string name() const
    {
        return _table.empty() ? _name : _table + "." + _name;
    }

    /*!
        \brief Returns unqualified name (without table name)
    */
    std::string unqualifiedName() const
    {
        return _name;
    }

    std::ptrdiff_t offset() const
    {
        return _offset;
    }

    SqlValue config() const
    {
        return _config;
    }

    std::uint32_t flags() const
    {
        return _flags;
    }

    std::size_t typeHash() const
    {
        return _typeHash;
    }

    std::string tableName() const
    {
        return _table;
    }

    void setTable(const std::string &tableName)
    {
        _table = tableName;
    }

    /**
     * \brief Return true if this cell could hold nullable value
     *
     * \return true if cell is nullable, false otherwise
     */
    bool isNullable() const
    {
        return _isNullable;
    }

    template <typename Struct>
    void deserialize(const char *value, Struct &node) const
    {
        if (_type)
        {
            dynamic_cast<Holder<Struct> &>(*_type).deserialize(value, node);
        }
    }

    template <typename Struct>
    void serialize(const Struct &node)
    {
        _value = dynamic_cast<Holder<Struct> &>(*_type).serialize(node);
    }

    const SqlValue& value() const
    {
        return _value;
    }

    /**
     * \brief Helper function used go calculate field offset
     *
     * \tparam Struct    Structure type
     * \tparam T         Field type
     * \param member     Structure member used to calculate offset within structure
     * \return size_t    Zero based offset for provided field
     */
    template <typename Struct, typename T>
    static size_t fieldOffset(T Struct::*member)
    {
        return (reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<Struct const volatile *>(0)->*member)));
    }

private:
    struct TypeHolder
    {
        virtual ~TypeHolder() = default;
    };
    template <typename Struct>
    class Holder : public TypeHolder
    {
    public:
        using SerFn = std::function<SqlValue(const Struct &)>;
        using DeserFn = std::function<void(const char *, Struct &)>;

        template <typename T>
        explicit Holder(T Struct::*member, const TypeConverter<T> &converter)
            : serialize([member, converter](const Struct &node) { return converter.from(node.*member); })
            , deserialize([member, converter](const char *value, Struct &node) { node.*member = converter.to(value); })
        {
        }

        Holder &operator=(const Holder &) = delete;
        Holder &operator=(Holder &&) = delete;

        SerFn serialize;
        DeserFn deserialize;
    };

private:
    std::size_t _typeHash{0};
    std::shared_ptr<TypeHolder> _type;
    std::ptrdiff_t _offset{0};
    std::string _name;
    std::string _table;
    SqlValue _config;
    SqlValue _value;
    std::uint32_t _flags{Flags::NONE};
    bool _isNullable = false; /// true if cell could contain nullable values
};

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_CELL_H_
