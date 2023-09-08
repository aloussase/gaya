#include <cassert>
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
      identifier.get_span(), identifier.get_span().to_string()
  );

  if (auto token = _lexer.next_token(); token) {
    return std::make_unique<ast::declaration_stmt>(
        std::move(ident), expression(token.value())
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
  case token_type::identifier: return call_expression(token);
  default: return primary_expression(token);
  }
}

ast::expression_ptr parser::function_expression(token lcurly)
{
  std::vector<ast::identifier> params;

  for (;;) {
    auto identifier = _lexer.next_token();
    if (!identifier || identifier->type() != token_type::identifier) {
      break;
    }

    auto span = identifier->get_span();
    params.emplace_back(span, span.to_string());

    auto comma = _lexer.next_token();
    if (!comma || comma->type() != token_type::comma) {
      break;
    }
  }

  if (!params.empty()) {
    auto arrow = _lexer.next_token();
    if (!arrow || arrow->type() != token_type::arrow) {
      parser_error(lcurly.get_span(), "Expected '=>' after function params");
    }
  }

  auto expr = _lexer.next_token();
  return std::make_unique<ast::function_expression>(
      std::move(params),
      expr ? expression(expr.value())
           : std::make_unique<ast::unit>(lcurly.get_span())
  );
}

ast::expression_ptr parser::call_expression(token identifier)
{
  // TODO: Not implemented
  assert(false && "not implemented");
}

ast::expression_ptr parser::primary_expression(token token)
{
  switch (token.type()) {
  case token_type::number:
    return std::make_unique<ast::number>(
        token.get_span(), std::stod(token.get_span().to_string())
    );
  case token_type::string:
    return std::make_unique<ast::string>(
        token.get_span(), token.get_span().to_string()
    );
  case token_type::identifier:
    return std::make_unique<ast::identifier>(
        token.get_span(), token.get_span().to_string()
    );
  default:
    parser_error(token.get_span(), "Invalid start of primary expression");
  }
}
