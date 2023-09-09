#include <cassert>

#include <fmt/core.h>

#include <eval.hpp>
#include <parser.hpp>
#include <span.hpp>

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
  return eval(env {}, std::move(ast));
}

object::object_ptr interpreter::eval(env env, ast::node_ptr ast) noexcept
{
  _scopes.push(env);
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
  return _scopes.top();
}

env& interpreter::current_env() noexcept
{
  return _scopes.top();
}

void interpreter::define(const std::string& k, object::object_ptr v) noexcept
{
  current_env().set(k, v);
}

void interpreter::begin_scope(env new_scope) noexcept
{
  _scopes.push(new_scope);
}

void interpreter::end_scope() noexcept
{
  _scopes.pop();
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
    define(ident, std::shared_ptr { std::move(value) });
  }

  return nullptr;
}

object::object_ptr interpreter::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
  expression_stmt.expr->accept(*this);
  return nullptr;
}

object::object_ptr interpreter::visit_call_expression(ast::call_expression& cexpr)
{
  auto o = cexpr.identifier->accept(*this);
  if (!o) return nullptr;

  if (!o->is_callable()) {
    _diagnostics.emplace_back(
        cexpr.identifier->_span, //
        "Tried to call non-callable",
        diagnostic::severity::error
    );
    return nullptr;
  }

  auto callable = std::static_pointer_cast<object::callable>(o);
  if (callable->arity() != cexpr.args.size()) {
    _diagnostics.emplace_back(
        cexpr.identifier->_span, //
        "Wrong number of arguments provided to callable",
        diagnostic::severity::error
    );
    return nullptr;
  }

  std::vector<object::object_ptr> args(cexpr.args.size());
  std::transform(
      cexpr.args.begin(), //
      cexpr.args.end(),
      args.begin(),
      [&](auto& expr) { return expr->accept(*this); }
  );

  return callable->call(*this, args);
}

object::object_ptr interpreter::visit_function_expression(ast::function_expression& fexpr)
{
  return std::make_unique<object::function>(
      fexpr._span, //
      fexpr.params,
      std::move(fexpr.body),
      env { std::make_unique<env>(current_env()) }
  );
}

object::object_ptr interpreter::visit_let_expression(ast::let_expression& let_expression)
{
  auto ident   = let_expression.ident->value;
  auto binding = let_expression.binding->accept(*this);
  if (!binding) return nullptr;

  begin_scope(env { std::make_shared<env>(current_env()) });
  define(ident, binding);

  auto result = let_expression.expr->accept(*this);

  end_scope();

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
  if (auto value = current_env().get(ident); value) {
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
