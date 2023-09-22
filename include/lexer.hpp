#pragma once

#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

#include <diagnostic.hpp>
#include <span.hpp>

enum class token_type {
    and_,
    arrow,
    back_arrow,
    cases,
    colon,
    colon_colon,
    comma,
    dash,
    diamond,
    do_,
    dot,
    end,
    equal,
    equal_equal,
    for_,
    given,
    greater_than,
    greater_than_eq,
    identifier,
    in,
    include,
    land,
    lcurly,
    less_than,
    less_than_eq,
    let,
    lnot,
    lor,
    lparen,
    lshift,
    not_,
    not_equals,
    number,
    or_,
    otherwise,
    perform,
    pipe,
    plus,
    rcurly,
    rparen,
    rshift,
    slash,
    star,
    string,
    thin_arrow,
    underscore,
    unit,
    when,
    while_,
    xor_,
};

struct token
{
    constexpr token(token_type type, class span span)
        : type { type }
        , span { span }
    {
    }

    token_type type;
    class span span;
};

class lexer
{
public:
    lexer(const char* source);

    [[nodiscard]] std::optional<token> next_token() noexcept;

    /**
     * Look at the next token without advancing the lexer.
     */
    [[nodiscard]] std::optional<token> peek_token() noexcept;

    [[nodiscard]] std::vector<diagnostic::diagnostic>
    diagnostics() const noexcept;

    void push_back(token) noexcept;

    template <typename Callback>
    void for_each(Callback f) noexcept
    {
        auto token = next_token();
        while (token)
        {
            f(token.value());
            token = next_token();
        }
    }

    [[nodiscard]] static bool is_keyword(const std::string&) noexcept;

private:
    std::optional<char> advance() noexcept;

    std::optional<char> peek() const noexcept;

    bool is_at_end() const noexcept;

    token mk_token(token_type) const noexcept;

    void lexer_error(const std::string& msg) noexcept;

    void lexer_hint(const std::string& msg) noexcept;

    [[nodiscard]] bool is_valid_identifier(char c) const noexcept;

    [[nodiscard]] bool is_digit(char c) const noexcept;

    [[nodiscard]] std::optional<token> colon_colon() noexcept;
    [[nodiscard]] std::optional<token> comment() noexcept;
    [[nodiscard]] std::optional<token> dash() noexcept;
    [[nodiscard]] std::optional<token> slash() noexcept;
    [[nodiscard]] std::optional<token> pipe() noexcept;
    [[nodiscard]] std::optional<token> less_than() noexcept;
    [[nodiscard]] std::optional<token> greater_than() noexcept;
    [[nodiscard]] std::optional<token> arrow() noexcept;
    [[nodiscard]] std::optional<token> number() noexcept;
    [[nodiscard]] std::optional<token> string() noexcept;
    [[nodiscard]] std::optional<token> identifier() noexcept;

    char* _current            = 0;
    char* _start              = 0;
    size_t _lineno            = 1;
    const char* _source       = nullptr;
    std::queue<token> _buffer = {};
    std::vector<diagnostic::diagnostic> _diagnostics;
    static std::unordered_map<std::string, token_type> _keywords;
};
