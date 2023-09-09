#pragma once

#include <ast_visitor.hpp>
#include <diagnostic.hpp>
#include <env.hpp>
#include <object.hpp>
#include <span.hpp>

namespace eval {

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

  object::object_ptr visit_program(ast::program&) override;
  object::object_ptr visit_declaration_stmt(ast::declaration_stmt&) override;
  object::object_ptr visit_expression_stmt(ast::expression_stmt&) override;
  object::object_ptr visit_call_expression(ast::call_expression&) override;
  object::object_ptr visit_function_expression(ast::function_expression&) override;
  object::object_ptr visit_let_expression(ast::let_expression&) override;
  object::object_ptr visit_number(ast::number&) override;
  object::object_ptr visit_string(ast::string&) override;
  object::object_ptr visit_identifier(ast::identifier&) override;
  object::object_ptr visit_unit(ast::unit&) override;

private:
  void undefined_identifier(span, const std::string&) noexcept;

  const char* _source = nullptr;
  std::vector<diagnostic::diagnostic> _diagnostics;
  env _current_env;
};

}
