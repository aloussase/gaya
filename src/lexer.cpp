#include <cstring>

#include <lexer.hpp>

lexer::lexer(const char* source)
    : _current { const_cast<char*>(source) }
    , _source { source }
{
}

std::optional<char> lexer::advance() noexcept
{
  if (is_at_end()) {
    return std::nullopt;
  }
  return *_current++;
}

std::optional<char> lexer::peek() const noexcept
{
  if (is_at_end()) {
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
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '.'
      || c == '_';
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
      span { _lineno, _start, _current }, //
      msg,
      diagnostic::severity::error
  );
}

void lexer::lexer_hint(const std::string& msg) noexcept
{
  _diagnostics.emplace_back(
      span { _lineno, _start, _current }, //
      msg,
      diagnostic::severity::hint
  );
}

std::optional<token> lexer::next_token() noexcept
{
  if (!_buffer.empty()) {
    auto token = _buffer.front();
    _buffer.pop();
    return token;
  }

  _start = _current;

  auto c = advance();
  if (!c) {
    return std::nullopt;
  }

  switch (c.value()) {
  case '{': return mk_token(token_type::lcurly);
  case '}': return mk_token(token_type::rcurly);
  case '(': return comment();
  case ')': return mk_token(token_type::rparen);
  case ',': return mk_token(token_type::comma);
  case ':': return colon_colon();
  case '=': return arrow();
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
  default: return is_valid_identifier(c.value()) ? identifier() : std::nullopt;
  }
}

std::optional<token> lexer::colon_colon() noexcept
{
  if (auto c = peek(); !c || c.value() != ':') {
    return std::nullopt;
  }
  advance();
  return mk_token(token_type::colon_colon);
}

std::optional<token> lexer::comment() noexcept
{
  if (auto c = peek(); c && c.value() != '*') {
    return mk_token(token_type::lparen);
  }

  advance();
  auto c = peek();
  for (;;) {
    if (!c) {
      lexer_error("Unterminated comment");
      return std::nullopt;
    }

    advance();

    if (c == '\n') _lineno++;
    if (c.value() == '*') {
      if (c = peek(); c && c.value() == ')') {
        advance();
        break;
      }
    }

    c = peek();
  }

  return next_token();
}

std::optional<token> lexer::arrow() noexcept
{
  if (auto c = peek(); !c || c.value() != '>') {
    return mk_token(token_type::equal);
  }
  advance();
  return mk_token(token_type::arrow);
}

std::optional<token> lexer::number() noexcept
{
  auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

  for (;;) {
    if (auto c = peek(); c && is_digit(c.value())) {
      advance();
    } else {
      break;
    }
  }

  return mk_token(token_type::number);
}

std::optional<token> lexer::string() noexcept
{
  for (;;) {
    if (auto c = peek(); c && c.value() != '"') {
      advance();
    } else if (c && c.value() == '"') {
      advance();
      break;
    } else {
      return std::nullopt;
    }
  }

  return mk_token(token_type::string);
}

std::optional<token> lexer::identifier() noexcept
{
  for (;;) {
    if (auto c = peek(); c && is_valid_identifier(c.value())) {
      advance();
    } else {
      break;
    }
  }

  // FIXME: This could be faster if we stored keywords in a hash map.

  if (strncmp(_start, "discard", _current - _start) == 0) {
    return mk_token(token_type::discard);
  } else if (strncmp(_start, "let", _current - _start) == 0) {
    return mk_token(token_type::let);
  } else if (strncmp(_start, "in", _current - _start) == 0) {
    return mk_token(token_type::in);
  } else {
    return mk_token(token_type::identifier);
  }
}
