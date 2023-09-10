#include <cassert>
#include <iostream>
#include <memory>

#include <ast.hpp>
#include <parser.hpp>

namespace gaya {

parser::parser(const char* source)
    : _lexer { source }
{
}

std::vector<diagnostic::diagnostic> parser::diagnostics() const noexcept
{
  return _diagnostics;
}

void parser::parser_error(span s, const std::string& message)
{
  _diagnostics.emplace_back(s, message, diagnostic::severity::error);
}

void parser::parser_hint(span s, const std::string& message)
{
  _diagnostics.emplace_back(s, message, diagnostic::severity::hint);
}

void parser::merge_diagnostics() noexcept
{
  for (const auto& diag : _lexer.diagnostics()) {
    _diagnostics.insert(_diagnostics.begin(), diag);
  }
}

bool parser::match(std::optional<token> t, token_type tt) const noexcept
{
  return t && t->type() == tt;
}

std::vector<token> parser::remaining_tokens() noexcept
{
  std::vector<token> tokens;
  for (;;) {
    auto token = _lexer.next_token();
    if (!token) break;

    tokens.push_back(token.value());
  }
  return tokens;
}

ast::node_ptr parser::parse() noexcept
{
  auto program   = std::make_unique<ast::program>();
  program->stmts = stmts();
  merge_diagnostics();
  return program;
}

ast::expression_ptr parser::parse_expression() noexcept
{
  auto token = _lexer.next_token();
  if (token) {
    return expression(token.value());
  }
  return nullptr;
}

std::vector<ast::stmt_ptr> parser::stmts() noexcept
{
  std::vector<ast::stmt_ptr> _stmts;

  for (;;) {
    if (auto stmt = parse_stmt(); stmt) {
      _stmts.push_back(std::move(stmt));
    } else {
      break;
    }
  }

  return _stmts;
}

ast::stmt_ptr parser::parse_stmt() noexcept
{
  auto token = _lexer.next_token();
  if (!token) {
    return nullptr;
  }

  switch (token->type()) {
  case token_type::identifier: return declaration_stmt(token.value());
  case token_type::discard: return expression_stmt(token.value());
  default:
    parser_error(token->get_span(), "Invalid start of statement");
    parser_hint(token->get_span(), "Only definitions and discard are valid top-level statements");
    return nullptr;
  }
}

ast::stmt_ptr parser::declaration_stmt(token identifier)
{
  auto colon_colon = _lexer.next_token();
  if (!colon_colon || colon_colon->type() != token_type::colon_colon) {
    parser_error(identifier.get_span(), "Expected a '::' after identifier");
    return nullptr;
  }

  auto ident = std::make_unique<ast::identifier>(
      identifier.get_span(), //
      identifier.get_span().to_string()
  );

  auto token = _lexer.next_token();
  if (!token) {
    parser_error(colon_colon->get_span(), "Expected an expression after '::'");
    return nullptr;
  }

  auto expr = expression(token.value());
  if (!expr) return nullptr;

  return std::make_unique<ast::declaration_stmt>(std::move(ident), std::move(expr));
}

ast::stmt_ptr parser::expression_stmt(token discard)
{
  auto token = _lexer.next_token();
  if (!token) {
    parser_error(discard.get_span(), "Expected an expression after discard");
    return nullptr;
  }

  auto expr = expression(token.value());
  if (!expr) return nullptr;

  return std::make_unique<ast::expression_stmt>(std::move(expr));
}

ast::expression_ptr parser::expression(token token)
{
  switch (token.type()) {
  case token_type::lcurly: return function_expression(token);
  case token_type::let: return let_expression(token);
  default: return call_expression(token);
  }
}

ast::expression_ptr parser::function_expression(token lcurly)
{
  std::vector<ast::identifier> params;

  for (;;) {
    auto identifier = _lexer.next_token();
    if (!identifier) {
      break;
    }

    if (identifier->type() != token_type::identifier) {
      _lexer.push_back(identifier.value());
      break;
    }

    params.emplace_back(
        identifier->get_span(), //
        identifier->get_span().to_string()
    );

    auto comma = _lexer.next_token();
    if (!comma) {
      break;
    }

    if (comma->type() != token_type::comma) {
      _lexer.push_back(comma.value());
      break;
    }
  }

  auto arrow = _lexer.next_token();
  if (!arrow || arrow->type() != token_type::arrow) {
    parser_error(lcurly.get_span(), "Expected '=>' after function params");
    return nullptr;
  }

  auto token = _lexer.next_token();
  if (!token) {
    parser_error(lcurly.get_span(), "Expected a '}' after function body");
    return nullptr;
  }

  if (token->type() == token_type::rcurly) {
    return std::make_unique<ast::unit>(token->get_span());
  }

  auto expr = expression(token.value());
  if (!expr) return nullptr;

  auto ret = std::make_unique<ast::function_expression>(
      lcurly.get_span(),
      std::move(params), //
      std::move(expr)
  );

  auto rcurly = _lexer.next_token();
  if (!rcurly || rcurly->type() != token_type::rcurly) {
    parser_error(lcurly.get_span(), "Expected a '}' after function body");
    return nullptr;
  }

  return ret;
}

ast::expression_ptr parser::call_expression(token tk)
{
  auto token = tk;
  auto expr  = primary_expression(token);
  if (!expr) return nullptr;

  for (;;) {
    auto token = _lexer.next_token();
    if (!token) break;

    if (!match(token, token_type::lparen)) {
      // Not a function call.
      _lexer.push_back(token.value());
      break;
    }

    std::vector<ast::expression_ptr> args;

    for (;;) {
      token = _lexer.next_token();
      if (!token) {
        parser_error(tk.get_span(), "Expected a ')' after opening '('");
        return nullptr;
      }

      if (match(token, token_type::rparen)) {
        _lexer.push_back(token.value());
        break;
      }

      auto arg = expression(token.value());
      if (!arg) return nullptr;

      args.push_back(std::move(arg));

      if (token = _lexer.next_token(); !match(token, token_type::comma)) {
        _lexer.push_back(token.value());
        break;
      }
    }

    if (token = _lexer.next_token(); !match(token, token_type::rparen)) {
      parser_error(tk.get_span(), "Missing ')' after function call");
      return nullptr;
    }

    expr = std::make_unique<ast::call_expression>(tk.get_span(), std::move(expr), std::move(args));
  }

  return expr;
}

ast::expression_ptr parser::let_expression(token let)
{
  auto ident = _lexer.next_token();
  if (!match(ident, token_type::identifier)) {
    parser_error(let.get_span(), "Expected an identifier after 'let'");
    return nullptr;
  }

  auto equal_sign = _lexer.next_token();
  if (!match(equal_sign, token_type::equal)) {
    parser_error(ident->get_span(), "Expected '=' after identifier in let expression");
    return nullptr;
  }

  auto token = _lexer.next_token();
  if (!token) {
    parser_error(equal_sign->get_span(), "Expected an expression after '='");
    return nullptr;
  }

  auto binding = expression(token.value());
  if (!binding) return nullptr;

  auto in = _lexer.next_token();
  if (!match(in, token_type::in)) {
    parser_error(token->get_span(), "Expected 'in' after expression in let expression");
    return nullptr;
  }

  token = _lexer.next_token();
  if (!token) {
    parser_error(in->get_span(), "Expected an expression after 'in'");
    return nullptr;
  }

  auto expr = expression(token.value());
  if (!expr) return nullptr;

  auto identifier = std::make_unique<ast::identifier>(
      ident->get_span(), //
      ident->get_span().to_string()
  );

  return std::make_unique<ast::let_expression>(
      std::move(identifier), //
      std::move(binding),
      std::move(expr)
  );
}

ast::expression_ptr parser::primary_expression(token token)
{
  switch (token.type()) {
  case token_type::number:
    return std::make_unique<ast::number>(
        token.get_span(), //
        std::stod(token.get_span().to_string())
    );
  case token_type::string:
    return std::make_unique<ast::string>(
        token.get_span(), //
        token.get_span().to_string()
    );
  case token_type::identifier:
    return std::make_unique<ast::identifier>(
        token.get_span(), //
        token.get_span().to_string()
    );
  default:
    parser_error(token.get_span(), "Invalid start of primary expression"); //
    return nullptr;
  }
}

}
