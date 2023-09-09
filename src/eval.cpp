#include <cassert>

#include <fmt/core.h>

#include <eval.hpp>
#include <parser.hpp>
#include <span.hpp>

#define BEGIN_SCOPE(env)           \
  auto __old_env__ = _current_env; \
  _current_env     = env;

#define END_SCOPE() _current_env = __old_env__;

namespace gaya::eval {

interpreter::interpreter(const char* source)
    : _source { source }
{
}

object::object_ptr interpreter::eval() noexcept
{
  auto p   = parser(_source);
  auto ast = p.parse();
  if (!p.diagnostics().empty()) {
    _diagnostics = p.diagnostics();
    return nullptr;
  }
  return eval(_current_env, std::move(ast));
}

object::object_ptr interpreter::eval(env env, ast::node_ptr ast) noexcept
{
  _current_env = env;
  return ast->accept(*this);
}

std::vector<diagnostic::diagnostic> interpreter::diagnostics() const noexcept
{
  return _diagnostics;
}

void interpreter::clear_diagnostics() noexcept
{
  _diagnostics.clear();
}

const env& interpreter::get_env() const noexcept
{
  return _current_env;
}

void interpreter::undefined_identifier(span s, const std::string& identifier) noexcept
{
  _diagnostics.emplace_back(
      s, //
      fmt::format("undefined identifier: {}", identifier),
      diagnostic::severity::error
  );
}

object::object_ptr interpreter::visit_program(ast::program& program)
{
  for (const auto& stmt : program.stmts) {
    stmt->accept(*this);
  }
  return nullptr;
}

object::object_ptr interpreter::visit_declaration_stmt(ast::declaration_stmt& declaration_stmt)
{
  auto ident = declaration_stmt._identifier->value;
  auto value = declaration_stmt.expression->accept(*this);

  if (value) {
    _current_env.set(ident, std::shared_ptr { std::move(value) });
  }

  return nullptr;
}

object::object_ptr interpreter::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
  expression_stmt.expr->accept(*this);
  return nullptr;
}

object::object_ptr interpreter::visit_call_expression(ast::call_expression&)
{
  assert(false && "not implemented");
  return nullptr;
}

object::object_ptr interpreter::visit_function_expression(ast::function_expression&)
{
  assert(false && "not implemented");
  return nullptr;
}

object::object_ptr interpreter::visit_let_expression(ast::let_expression& let_expression)
{
  auto ident   = let_expression.ident->value;
  auto binding = let_expression.binding->accept(*this);
  if (!binding) {
    return nullptr;
  }

  auto let_env = env { _current_env };

  BEGIN_SCOPE(let_env);
  auto result = let_expression.expr->accept(*this);
  END_SCOPE();

  return result;
}

object::object_ptr interpreter::visit_number(ast::number& n)
{
  return std::make_shared<object::number>(n._span, n.value);
}

object::object_ptr interpreter::visit_string(ast::string& s)
{
  return std::make_shared<object::string>(s._span, s.value);
}

object::object_ptr interpreter::visit_identifier(ast::identifier& identifier)
{
  auto ident = identifier.value;
  if (auto value = _current_env.get(ident); value) {
    return value;
  }
  undefined_identifier(identifier._span, identifier.value);
  return nullptr;
}

object::object_ptr interpreter::visit_unit(ast::unit& u)
{
  return std::make_shared<object::unit>(u._span);
}

}
