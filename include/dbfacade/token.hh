#ifndef SOFTEQ_DBFACADE_ATOM_H_
#define SOFTEQ_DBFACADE_ATOM_H_

#include <string>
#include <vector>

#include "sqlvalue.hh"

namespace softeq
{
namespace db
{
/*!
    \brief Class that represent a token which can be either a SqlValue or a string
*/
class Token
{
public:
    /*!
        \brief Constructs a token from a string. 
        It will be represented in a resulting statement exactly as the string specified.
        \param value the string
    */
    explicit Token(const std::string &value)
        : _isValue(false)
        , _text(value)
    {
    }

    /*!
        \brief Constructs a token from a SqlValue object. 
        It will be represented in a resulted statement as a placeholder to bind the value.
        \param value the string
    */
    explicit Token(const SqlValue &value)
        : _isValue(true)
        , _value(value)
    {
    }

    /*!
        \brief Extracts all values from a vector of tokens
        \param tokens a vector of tokens
        \returns a vector of SqlValue
    */
    static std::vector<SqlValue> bindingParameters(const std::vector<Token> &tokens)
    {
        std::vector<SqlValue> retval;

        for (auto &token : tokens)
        {
            if (token._isValue)
            {
                retval.push_back(token._value);
            }
        }
        return retval;
    }

    bool isValue() const
    {
        return _isValue;
    }

    const std::string text() const
    {
        return _text;
    }

    const SqlValue &value() const
    {
        return _value;
    }

private:
    bool _isValue;
    std::string _text;
    SqlValue _value;
};

/*!
    \brief Shift operator that adds a token to a vector of tokens.
    Its purpose is to increase code readability.
    \param tokens a vector of tokens
    \param newToken a token to add
    \returns a modified tokens parameter
*/
template <typename TokenT, typename std::enable_if<std::is_constructible<Token, TokenT>::value>::type * = nullptr>
inline std::vector<Token> &operator<<(std::vector<Token> &tokens, TokenT &&newToken)
{
    tokens.emplace_back(std::forward<TokenT>(newToken));
    return tokens;
}

/*!
    \brief Shift operator that appends a vector of tokens to another vector of tokens.
    Its purpose is to increase code readability.
    \param tokens a vector of tokens
    \param newTokens a vector to append
    \returns a modified tokens parameter
*/
inline std::vector<Token> &operator<<(std::vector<Token> &tokens, const std::vector<Token> &newTokens)
{
    tokens.insert(std::end(tokens), std::begin(newTokens), std::end(newTokens));
    return tokens;
}

} // namespace db
} // namespace softeq

#endif //