#include <cstring>

#include <lexer.hpp>

std::unordered_map<std::string, token_type> lexer::_keywords = {
    { "let", token_type::let },
    { "in", token_type::in },
    { "do", token_type::do_ },
    { "unit", token_type::unit },
    { "cases", token_type::cases },
    { "given", token_type::given },
    { "otherwise", token_type::otherwise },
    { "end", token_type::end },
    { "while", token_type::while_ },
    { "for", token_type::for_ },
    { "not", token_type::not_ },
    { "perform", token_type::perform },
    { "and", token_type::and_ },
    { "or", token_type::or_ },
    { "include", token_type::include },
    { "when", token_type::when },
    { "of", token_type::of },
    { "type", token_type::type },
    { "with", token_type::with },
    { "foreign", token_type::foreign },
};

lexer::lexer(const char* source)
    : _current { const_cast<char*>(source) }
    , _source { source }
{
}

bool lexer::is_keyword(const std::string& w) noexcept
{
    return _keywords.contains(w);
}

bool lexer::is_keyword(const char* w) noexcept
{
    return is_keyword(std::string(w));
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
    return token { type, span { _source, _lineno, _start, _current } };
}

bool lexer::is_valid_identifier(char c) const noexcept
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z') || c == '.' || c == '_';
}

bool lexer::is_digit(char c) const noexcept
{
    return c >= '0' && c <= '9';
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
        span { _source, _lineno, _start, _current },
        msg,
        diagnostic::severity::error);
}

void lexer::lexer_hint(const std::string& msg) noexcept
{
    _diagnostics.emplace_back(
        span { _source, _lineno, _start, _current },
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
    case '_': return mk_token(token_type::underscore);
    case '&':
    {
        if (auto c = peek(); c && is_valid_identifier(*c))
        {
            return mk_token(token_type::ampersand);
        }
        return mk_token(token_type::land);
    }
    case '-': return dash();
    case '/': return slash();
    case '|': return pipe();
    case '^': return mk_token(token_type::xor_);
    case '~': return mk_token(token_type::lnot);
    case '.': return mk_token(token_type::dot);
    case '@': return mk_token(token_type::at);
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
        if (is_valid_identifier(c.value()) && c.value() != '.')
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

std::optional<token> lexer::peek_token() noexcept
{
    if (!_buffer.empty())
    {
        return _buffer.back();
    }

    auto token = next_token();
    if (token)
    {
        _buffer.push(token.value());
    }

    return token;
}

std::optional<token> lexer::colon_colon() noexcept
{
    if (auto c = peek(); !c || c.value() != ':')
    {
        return mk_token(token_type::colon);
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

std::optional<token> lexer::slash() noexcept
{
    if (auto c = peek(); c && c.value() == '=')
    {
        advance();
        return mk_token(token_type::not_equals);
    }
    return mk_token(token_type::slash);
}

std::optional<token> lexer::pipe() noexcept
{
    if (auto c = peek(); c && c.value() == '>')
    {
        advance();
        return mk_token(token_type::pipe);
    }

    return mk_token(token_type::lor);
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

        if (c.value() == '>')
        {
            advance();
            return mk_token(token_type::diamond);
        }

        if (c.value() == '<')
        {
            advance();
            return mk_token(token_type::lshift);
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

    if (auto c = peek(); c && c.value() == '>')
    {
        advance();
        return mk_token(token_type::rshift);
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

std::optional<token> lexer::dash() noexcept
{
    if (auto c = peek(); c && is_digit(c.value()))
    {
        advance();
        return number();
    }

    if (auto c = peek(); c && c == '>')
    {
        advance();
        return mk_token(token_type::thin_arrow);
    }

    return mk_token(token_type::dash);
}

std::optional<token> lexer::number() noexcept
{
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

    if (auto c = peek(); c && c == 'x')
    {
        /* A hex literal. */
        advance();

        auto is_valid_hex_digit = [](char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
                || (c >= 'A' && c <= 'F');
        };

        for (;;)
        {
            if (auto c = peek(); c && is_valid_hex_digit(c.value()))
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

    if (auto c = peek(); c && c == '.'
        && !(*(_current + 1) == '\0' || !is_digit(*(_current + 1))))
    {
        /* Parse floating point literal. */
        auto lex_at_least_one_decimal = false;
        advance();
        for (;;)
        {
            if (auto c = peek(); c && is_digit(c.value()))
            {
                lex_at_least_one_decimal = true;
                advance();
            }
            else
            {
                break;
            }
        }

        if (!lex_at_least_one_decimal)
        {
            lexer_error("Expected at least one decimal point after '.' in "
                        "numeric literal");
            return std::nullopt;
        }
    }

    return mk_token(token_type::number);
}

std::optional<token> lexer::string() noexcept
{
    char previous = '\0';

    for (;;)
    {
        auto c = peek();
        if (!c) break;

        if (c && (c.value() != '"' || (c.value() == '"' && previous == '\\')))
        {
            previous = c.value();
            advance();
        }
        else if (c && c.value() == '"')
        {
            break;
        }
    }

    if (auto c = peek(); !c || c != '"')
    {
        lexer_error("Unterminated string literal");
        return std::nullopt;
    }

    advance();

    return token {
        token_type::string,
        span { _source, _lineno, _start + 1, _current - 1 },
    };
}

std::optional<token> lexer::identifier() noexcept
{
    for (;;)
    {
        if (auto c = peek(); c && is_valid_identifier(c.value()))
        {
            /*
             * We need to check if the dot is being used as part of an
             * identifier or as a statement terminator.
             */
            if (c.value() == '.'
                && (*(_current + 1) == '\0'
                    || !is_valid_identifier(*(_current + 1))))
            {
                /* Here we're using it as a statement terminator. */
                break;
            }

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
