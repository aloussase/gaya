#pragma once

#include <stack>

#include <ast_visitor.hpp>
#include <diagnostic.hpp>
#include <env.hpp>
#include <object.hpp>
#include <span.hpp>

namespace gaya::eval {

class interpreter final : public ast::ast_visitor {
public:
  interpreter(const char* source);

  [[nodiscard]] object::object_ptr eval() noexcept;

  /// This one is useful for REPLs.
  [[nodiscard]] object::object_ptr eval(env, ast::node_ptr) noexcept;

  [[nodiscard]] std::vector<diagnostic::diagnostic> diagnostics() const noexcept;

  /// Remove all diagnostics from this interpreter.
  void clear_diagnostics() noexcept;

  /// Get the interpreter's environment.
  [[nodiscard]] const env& get_env() const noexcept;

  /// Define a new symbol in the current scope.
  void define(const std::string&, object::object_ptr) noexcept;

  /// Begin a new scope.
  void begin_scope(env new_scope) noexcept;

  /// End a previous scope.
  void end_scope() noexcept;

  object::object_ptr visit_program(ast::program&) override;
  object::object_ptr visit_declaration_stmt(ast::declaration_stmt&) override;
  object::object_ptr visit_expression_stmt(ast::expression_stmt&) override;
  object::object_ptr visit_do_expression(ast::do_expression&) override;
  object::object_ptr visit_call_expression(ast::call_expression&) override;
  object::object_ptr visit_function_expression(ast::function_expression&) override;
  object::object_ptr visit_let_expression(ast::let_expression&) override;
  object::object_ptr visit_number(ast::number&) override;
  object::object_ptr visit_string(ast::string&) override;
  object::object_ptr visit_identifier(ast::identifier&) override;
  object::object_ptr visit_unit(ast::unit&) override;

private:
  [[nodiscard]] env& current_env() noexcept;

  void interp_error(span, const std::string& msg);
  void interp_hint(span, const std::string& hint);

  const char* _source = nullptr;
  std::vector<diagnostic::diagnostic> _diagnostics;
  std::stack<env> _scopes;
};

}
