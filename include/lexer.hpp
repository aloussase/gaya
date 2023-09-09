#pragma once

#include <optional>
#include <queue>
#include <vector>

#include <span.hpp>

enum class token_type {
  arrow,
  colon_colon,
  comma,
  discard,
  equal,
  identifier,
  in,
  lcurly,
  let,
  lparen,
  number,
  rcurly,
  rparen,
  string,
};

class token {
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

class lexer {
public:
  lexer(const char* source);

  std::optional<token> next_token() noexcept;

  void push_back(token) noexcept;

  template <typename Callback>
  void for_each(Callback f) noexcept
  {
    auto token = next_token();
    while (token) {
      f(token.value());
      token = next_token();
    }
  }

private:
  std::optional<char> advance() noexcept;

  std::optional<char> peek() const noexcept;

  bool is_at_end() const noexcept;

  token mk_token(token_type) const noexcept;

  [[nodiscard]] bool is_valid_identifier(char c) const noexcept;

  [[nodiscard]] std::optional<token> colon_colon() noexcept;
  [[nodiscard]] std::optional<token> arrow() noexcept;
  [[nodiscard]] std::optional<token> number() noexcept;
  [[nodiscard]] std::optional<token> string() noexcept;
  [[nodiscard]] std::optional<token> identifier() noexcept;

  char* _current            = 0;
  char* _start              = 0;
  size_t _lineno            = 1;
  const char* _source       = nullptr;
  std::queue<token> _buffer = {};
};
