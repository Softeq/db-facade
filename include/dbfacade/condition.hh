#ifndef SOFTEQ_DBFACADE_CONDITION_H_
#define SOFTEQ_DBFACADE_CONDITION_H_

#include <sstream>
#include <iterator>

#include "tablescheme.hh"
#include "token.hh"

namespace softeq
{
namespace db
{
namespace
{
// Enumeration of SQL operators

enum class Operator
{
    EQ,
    NEQ,
    LT,
    GT,
    LTE,
    GTE,
    AND,
    OR,
    BETWEEN,
    LIKE,
    IN
};

template <typename StreamT>
inline StreamT &operator<<(StreamT &out, Operator op)
{
    switch (op)
    {
    case Operator::EQ:
        return out << "=";
    case Operator::NEQ:
        return out << "<>";
    case Operator::LT:
        return out << "<";
    case Operator::GT:
        return out << ">";
    case Operator::LTE:
        return out << "<=";
    case Operator::GTE:
        return out << ">=";
    case Operator::AND:
        return out << "AND";
    case Operator::OR:
        return out << "OR";
    case Operator::BETWEEN:
        return out << "BETWEEN";
    case Operator::LIKE:
        return out << "LIKE";
    case Operator::IN:
        return out << "IN";
    }
    return out;
}

// Helpers for template magic
template <typename... Ts>
using void_type = void;

template <typename T, typename = void>
struct has_iterator : std::false_type
{
};

template <typename T>
struct has_iterator<T, void_type<typename T::iterator>> : std::true_type
{
};
} // namespace

/*!
   \brief A class that helps building SQL conditions which usually goes after WHERE or ON clauses
 */
class Condition
{
    std::vector<Token> _tokens;

public:
    struct WithoutParenthesis
    {
    }; // empty struct to use overload mechanism

    Condition()
    {
    }

    /*!
        \brief Construct a condition out of a cell (column)
        We might want to remove it in future since I do not think it should be required.
        \param value a cell
     */
    explicit Condition(const Cell &value)
    {
        _tokens << value.name();
    }

    /*!
        \brief Construct out of std::string and const char*
        Such values will be enclosed in single quotes in a resulting SQL expression.
        \param value a string
     */
    template <typename Type,
              typename std::enable_if<std::is_same<Type, std::string>::value || std::is_same<Type, const char *>::value,
                                      Type>::type * = nullptr>
    Condition(Type value)
    {
        _tokens << SqlValue(std::string(std::move(value)));
    }

    /*!
        \brief Construct out of iterable objects, like std::vector (but not std::string)
        \param value an iterable object
     */
    template <typename Container, typename std::enable_if<has_iterator<Container>::value, Container>::type * = nullptr,
              typename std::enable_if<std::is_same<typename Container::value_type, Condition>::value>::type * = nullptr>
    Condition(const Container &value)
    {
        if (value.begin() != value.end())
        {
            _tokens << "(";

            for (auto iter = value.begin(); iter != value.end(); ++iter)
            {
                _tokens << iter->_tokens;

                if (std::next(iter) != value.end())
                {
                    _tokens << ", ";
                }
            }

            _tokens << ")";
        }
    }

    /*!
        \brief Construct a condition out of a SqlValue
        \param value the value
     */
    Condition(const SqlValue &value)
    {
        _tokens << value;
    }

    /*!
        \brief Construct out of objects of other types but ones described above
        \param value the value
     */
    template <typename Type,
              typename std::enable_if<!has_iterator<Type>::value && !std::is_same<Type, const char *>::value,
                                      Type>::type * = nullptr>
    Condition(Type value)
    {
        _tokens << type_serializers::serialize<Type>::from(value);
    }

    /*!
        \brief Construct a complex expression condition (e.g.'a AND b').
        By default, all expressions are parenthesized, but this constructor does not. It is used for BETWEEN .. AND
        condition.

        \param op SQL operator
        \param lvalue left-hand-side of an expression
        \param rvalue right-hand side of an expression
     */
    Condition(Operator op, const Condition &lvalue, const Condition &rvalue, WithoutParenthesis)
    {
        _tokens << lvalue._tokens << " " << op << " " << rvalue._tokens;
    }

    /*!
        \brief Construct a complex expression condition (e.g.'a AND b').

        \param op SQL operator
        \param lvalue left-hand-side of an expression
        \param rvalue right-hand side of an expression
     */
    Condition(Operator op, const Condition &lvalue, const Condition &rvalue)
        : Condition(op, lvalue, rvalue, WithoutParenthesis{})
    {
        _tokens.emplace(_tokens.begin(), "("); // place into the beginning
        _tokens << ")";
    }

    /*!
        \brief Return a vector of tokens of the expression.
     */
    const std::vector<Token> &tokens() const
    {
        return _tokens;
    }

    /*!
        \brief check is a condition is not empty i.e. if it has been specified.
        \return true if the condition is not empty, false otherwise
     */
    bool hasValue() const
    {
        return !_tokens.empty();
    }

    // Helper operators for building SQL expressions
    // e.g. field(&table::field) > 10 || field(&table::field).between(2, 5)

    template <typename T>
    Condition operator&&(T value)
    {
        return Condition{Operator::AND, *this, Condition{value}};
    }
    template <typename T>
    Condition operator||(T value)
    {
        return Condition{Operator::OR, *this, Condition{value}};
    }
    template <typename T>
    Condition operator==(T value)
    {
        return Condition{Operator::EQ, *this, Condition{value}};
    }
    template <typename T>
    Condition operator!=(T value)
    {
        return Condition{Operator::NEQ, *this, Condition{value}};
    }
    template <typename T>
    Condition operator>=(T value)
    {
        return Condition{Operator::GTE, *this, Condition{value}};
    }
    template <typename T>
    Condition operator<=(T value)
    {
        return Condition{Operator::LTE, *this, Condition{value}};
    }
    template <typename T>
    Condition operator<(T value)
    {
        return Condition{Operator::LT, *this, Condition{value}};
    }
    template <typename T>
    Condition operator>(T value)
    {
        return Condition{Operator::GT, *this, Condition{value}};
    }
    Condition in(const std::initializer_list<Condition> &list)
    {
        return Condition{Operator::IN, *this, Condition{list}};
    }
    template <typename Low, typename High>
    Condition between(Low low, High high) const
    {
        return Condition{Operator::BETWEEN, *this,
                         Condition{Operator::AND, Condition{low}, Condition{high}, WithoutParenthesis{}}};
    }
    Condition like(const std::string &pattern) const
    {
        return Condition{Operator::LIKE, *this, Condition{pattern}};
    }
};

/*!
    \brief Creates a condition out of a single Cell.
    \param member C++ struct member corresponding to the BD cell
 */
template <typename Struct, typename T>
Condition field(T Struct::*member)
{
    return Condition{CellMaker(member)()};
}

} // namespace db
} // namespace softeq

#endif // SOFTEQ_DBFACADE_CONDITION_H_
