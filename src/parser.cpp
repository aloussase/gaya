#include <cassert>
#include <iostream>
#include <memory>

#include <ast.hpp>
#include <parser.hpp>

parser::parser(const char* source)
    : _source { source }
    , _lexer { source }
{
}

std::vector<diagnostic::diagnostic> parser::diagnostics() const noexcept
{
  return _diagnostics;
}

void parser::parser_error(span s, const std::string& message)
{
  _diagnostics.emplace_back(s, message, diagnostic::severity::error);
  throw synchronize {};
}

bool parser::match(std::optional<token> t, token_type tt) const noexcept
{
  return t && t->type() == tt;
}

ast::node_ptr parser::parse() noexcept
{
  auto program   = std::make_unique<ast::program>();
  program->stmts = stmts();
  return program;
}

std::vector<ast::stmt_ptr> parser::stmts() noexcept
{
  std::vector<ast::stmt_ptr> _stmts;

  for (;;) {
    auto token = _lexer.next_token();
    if (!token) {
      return _stmts;
    }

    try {
      switch (token->type()) {
      case token_type::identifier:
        _stmts.push_back(declaration_stmt(token.value()));
        break;
      case token_type::discard:
        _stmts.push_back(expression_stmt(token.value()));
        break;
      default: parser_error(token->get_span(), "Invalid top-level expression");
      }
    } catch (const synchronize&) {
      for (;;) {
        auto token = _lexer.next_token();
        if (!token) break;
        if (token->type() == token_type::identifier) break;
        if (token->type() == token_type::discard) break;
      }
    }
  }

  return _stmts;
}

ast::stmt_ptr parser::declaration_stmt(token identifier)
{
  auto colon_colon = _lexer.next_token();
  if (!colon_colon || colon_colon->type() != token_type::colon_colon) {
    parser_error(identifier.get_span(), "Expected a '::' after identifier");
  }

  auto ident = std::make_unique<ast::identifier>(
      identifier.get_span(), //
      identifier.get_span().to_string()
  );

  if (auto token = _lexer.next_token(); token) {
    return std::make_unique<ast::declaration_stmt>(
        std::move(ident), //
        expression(token.value())
    );
  } else {
    parser_error(colon_colon->get_span(), "Expected an expression after '::'");
  }
}

ast::stmt_ptr parser::expression_stmt(token discard)
{
  if (auto token = _lexer.next_token(); token) {
    return std::make_unique<ast::expression_stmt>(expression(token.value()));
  } else {
    parser_error(discard.get_span(), "Expected an expression after discard");
  }
}

ast::expression_ptr parser::expression(token token)
{
  switch (token.type()) {
  case token_type::lcurly: return function_expression(token);
  case token_type::let: return let_expression(token);
  case token_type::identifier: return call_expression(token);
  default: return primary_expression(token);
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
  }

  auto token = _lexer.next_token();
  if (!token) {
    parser_error(lcurly.get_span(), "Expected a '}' after function body");
  }

  if (token->type() == token_type::rcurly) {
    return std::make_unique<ast::unit>(token->get_span());
  }

  auto ret = std::make_unique<ast::function_expression>(
      std::move(params), //
      expression(token.value())
  );

  auto rcurly = _lexer.next_token();
  if (!rcurly || rcurly->type() != token_type::rcurly) {
    parser_error(lcurly.get_span(), "Expected a '}' after function body");
  }

  return ret;
}

ast::expression_ptr parser::call_expression(token identifier)
{
  auto lparen = _lexer.next_token();
  if (!lparen || lparen->type() != token_type::lparen) {
    if (lparen) {
      _lexer.push_back(lparen.value());
    }
    return std::make_unique<ast::identifier>(
        identifier.get_span(), //
        identifier.get_span().to_string()
    );
  }

  std::vector<ast::expression_ptr> args;

  for (;;) {
    auto token = _lexer.next_token();
    if (!token) {
      break;
    }

    if (token->type() == token_type::rparen) {
      // Empty args list.
      _lexer.push_back(token.value());
      break;
    }

    args.push_back(expression(token.value()));

    auto comma = _lexer.next_token();
    if (!comma) {
      parser_error(token->get_span(), "Expected a ')' after arguments");
    }

    if (comma->type() != token_type::comma) {
      _lexer.push_back(comma.value());
      break;
    }
  }

  auto ret = std::make_unique<ast::call_expression>(
      std::make_unique<ast::identifier>(
          identifier.get_span(), //
          identifier.get_span().to_string()
      ), //
      std::move(args)
  );

  auto rparen = _lexer.next_token();
  if (!rparen || rparen->type() != token_type::rparen) {
    parser_error(
        identifier.get_span(), //
        "Expected ')' after argument list"
    );
  }

  return ret;
}

ast::expression_ptr parser::let_expression(token let)
{
  auto ident = _lexer.next_token();
  if (!match(ident, token_type::identifier)) {
    parser_error(let.get_span(), "Expected an identifier after 'let'");
  }

  auto equal_sign = _lexer.next_token();
  if (!match(equal_sign, token_type::equal)) {
    parser_error(
        ident->get_span(), //
        "Expected '=' after identifier in let expression"
    );
  }

  auto token = _lexer.next_token();
  if (!token) {
    parser_error(equal_sign->get_span(), "Expected an expression after '='");
  }

  auto binding = expression(token.value());

  auto in = _lexer.next_token();
  if (!match(in, token_type::in)) {
    parser_error(
        token->get_span(), //
        "Expected 'in' after expression in let expression"
    );
  }

  token = _lexer.next_token();
  if (!token) {
    parser_error(in->get_span(), "Expected an expression after 'in'");
  }

  auto expr = expression(token.value());

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
    parser_error(token.get_span(), "Invalid start of primary expression");
  }
}
