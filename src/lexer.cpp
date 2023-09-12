#include <cstring>

#include <lexer.hpp>

lexer::lexer(const char* source)
    : _current { const_cast<char*>(source) }
    , _source { source }
{
    _keywords.insert({ "discard", token_type::discard });
    _keywords.insert({ "let", token_type::let });
    _keywords.insert({ "in", token_type::in });
    _keywords.insert({ "do", token_type::do_ });
    _keywords.insert({ "done", token_type::done });
    _keywords.insert({ "unit", token_type::unit });
    _keywords.insert({ "cases", token_type::cases });
    _keywords.insert({ "given", token_type::given });
    _keywords.insert({ "otherwise", token_type::otherwise });
    _keywords.insert({ "end", token_type::end });
}

std::optional<char> lexer::advance() noexcept
{
    if (is_at_end())
    {
        return std::nullopt;
    }
    return *_current++;
}

std::optional<char> lexer::peek() const noexcept
{
    if (is_at_end())
    {
        return std::nullopt;
    }
    return *_current;
}

bool lexer::is_at_end() const noexcept
{
    return *_current == '\0';
}

token lexer::mk_token(token_type type) const noexcept
{
    return token { type, span { _lineno, _start, _current } };
}

bool lexer::is_valid_identifier(char c) const noexcept
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z') || c == '.' || c == '_';
}

void lexer::push_back(token token) noexcept
{
    _buffer.push(token);
}

std::vector<diagnostic::diagnostic> lexer::diagnostics() const noexcept
{
    return _diagnostics;
}

void lexer::lexer_error(const std::string& msg) noexcept
{
    _diagnostics.emplace_back(
        span { _lineno, _start, _current },
        msg,
        diagnostic::severity::error);
}

void lexer::lexer_hint(const std::string& msg) noexcept
{
    _diagnostics.emplace_back(
        span { _lineno, _start, _current },
        msg,
        diagnostic::severity::hint);
}

std::optional<token> lexer::next_token() noexcept
{
    if (!_buffer.empty())
    {
        auto token = _buffer.front();
        _buffer.pop();
        return token;
    }

    _start = _current;

    auto c = advance();
    if (!c)
    {
        return std::nullopt;
    }

    switch (c.value())
    {
    case '{': return mk_token(token_type::lcurly);
    case '}': return mk_token(token_type::rcurly);
    case '(': return comment();
    case ')': return mk_token(token_type::rparen);
    case ',': return mk_token(token_type::comma);
    case ':': return colon_colon();
    case '=': return arrow();
    case '>': return greater_than();
    case '<': return less_than();
    case '+': return mk_token(token_type::plus);
    case '*': return mk_token(token_type::star);
    case '-': return mk_token(token_type::dash);
    case '/': return mk_token(token_type::slash);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': return number();
    case '"': return string();
    case '\n': _lineno += 1; return next_token();
    case ' ': return next_token();
    default:
        if (is_valid_identifier(c.value()))
        {
            return identifier();
        }
        else
        {
            lexer_error("Invalid token");
            return std::nullopt;
        }
    }
}

std::optional<token> lexer::colon_colon() noexcept
{
    if (auto c = peek(); !c || c.value() != ':')
    {
        lexer_error("Expected ':' after first ':'");
        return std::nullopt;
    }
    advance();
    return mk_token(token_type::colon_colon);
}

std::optional<token> lexer::comment() noexcept
{
    if (auto c = peek(); c && c.value() != '*')
    {
        return mk_token(token_type::lparen);
    }

    advance();
    auto c = peek();
    for (;;)
    {
        if (!c)
        {
            lexer_error("Unterminated comment");
            return std::nullopt;
        }

        advance();

        if (c == '\n') _lineno++;
        if (c.value() == '*')
        {
            if (c = peek(); c && c.value() == ')')
            {
                advance();
                break;
            }
        }

        c = peek();
    }

    return next_token();
}

std::optional<token> lexer::less_than() noexcept
{
    if (auto c = peek(); c)
    {
        if (c.value() == '=')
        {
            advance();
            return mk_token(token_type::less_than_eq);
        }

        if (c.value() == '-')
        {
            advance();
            return mk_token(token_type::back_arrow);
        }
    }

    return mk_token(token_type::less_than);
}

std::optional<token> lexer::greater_than() noexcept
{
    if (auto c = peek(); c && c.value() == '=')
    {
        advance();
        return mk_token(token_type::greater_than_eq);
    }

    return mk_token(token_type::greater_than);
}

std::optional<token> lexer::arrow() noexcept
{
    auto c = peek();

    if (c && (c.value() != '>' || c.value() != '='))
    {
        advance();
    }

    if (c && c.value() == '>') return mk_token(token_type::arrow);
    if (c && c.value() == '=') return mk_token(token_type::equal_equal);

    return mk_token(token_type::equal);
}

std::optional<token> lexer::number() noexcept
{
    auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

    // TODO: Lex floating point literals.

    for (;;)
    {
        if (auto c = peek(); c && is_digit(c.value()))
        {
            advance();
        }
        else
        {
            break;
        }
    }

    return mk_token(token_type::number);
}

std::optional<token> lexer::string() noexcept
{
    for (;;)
    {
        if (auto c = peek(); c && c.value() != '"')
        {
            advance();
        }
        else if (c && c.value() == '"')
        {
            advance();
            break;
        }
        else
        {
            lexer_error("Unterminated string literal");
            return std::nullopt;
        }
    }

    return token {
        token_type::string,
        span { _lineno, _start + 1, _current - 1 },
    };
}

std::optional<token> lexer::identifier() noexcept
{
    for (;;)
    {
        if (auto c = peek(); c && is_valid_identifier(c.value()))
        {
            advance();
        }
        else
        {
            break;
        }
    }

    if (auto* lexeme = strndup(_start, _current - _start);
        _keywords.contains(lexeme))
    {
        auto tt = _keywords[lexeme];
        free(lexeme);
        return mk_token(tt);
    }
    else
    {
        free(lexeme);
        return mk_token(token_type::identifier);
    }
}
