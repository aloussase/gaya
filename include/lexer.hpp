#pragma once

#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

#include <diagnostic.hpp>
#include <span.hpp>

enum class token_type {
    arrow,
    back_arrow,
    cases,
    colon_colon,
    comma,
    dash,
    discard,
    do_,
    done,
    end,
    equal,
    equal_equal,
    given,
    greater_than,
    greater_than_eq,
    identifier,
    in,
    lcurly,
    less_than,
    less_than_eq,
    let,
    lparen,
    not_,
    number,
    otherwise,
    plus,
    rcurly,
    rparen,
    slash,
    star,
    string,
    unit,
    while_,
};

class token
{
  public:
    constexpr token(token_type type, span span)
        : _type { type }
        , _span { span }
    {
    }

    [[nodiscard]] constexpr token_type type() const noexcept
    {
        return _type;
    }

    [[nodiscard]] constexpr span get_span() const noexcept
    {
        return _span;
    }

  private:
    token_type _type;
    span _span;
};

class lexer
{
  public:
    lexer(const char* source);

    [[nodiscard]] std::optional<token> next_token() noexcept;

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
    std::unordered_map<std::string, token_type> _keywords;
};
