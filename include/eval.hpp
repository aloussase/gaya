#pragma once

#include <stack>

#include <ast_visitor.hpp>
#include <diagnostic.hpp>
#include <env.hpp>
#include <object.hpp>
#include <span.hpp>

namespace gaya::eval
{

namespace o = object;

class interpreter final : public ast::ast_visitor
{
  public:
    interpreter(const char* source);

    [[nodiscard]] object::object_ptr eval() noexcept;

    /// This one is useful for REPLs.
    [[nodiscard]] object::object_ptr eval(env, ast::node_ptr) noexcept;

    [[nodiscard]] std::vector<diagnostic::diagnostic>
    diagnostics() const noexcept;

    /// Remove all diagnostics from this interpreter.
    void clear_diagnostics() noexcept;

    /// Get the interpreter's environment.
    [[nodiscard]] const env& get_env() const noexcept;

    /// Define a new symbol in the current scope.
    void define(const key&, object::object_ptr) noexcept;

    /// Begin a new scope.
    void begin_scope(env new_scope) noexcept;

    /// End a previous scope.
    void end_scope() noexcept;

    /// Add an interpreter error.
    void interp_error(span, const std::string& msg);

    /// Add an interpreter hint.
    void interp_hint(span, const std::string& hint);

    o::object_ptr visit_program(ast::program&) override;
    o::object_ptr visit_declaration_stmt(ast::declaration_stmt&) override;
    o::object_ptr visit_expression_stmt(ast::expression_stmt&) override;
    o::object_ptr visit_assignment_stmt(ast::assignment_stmt&) override;
    o::object_ptr visit_while_stmt(ast::WhileStmt&) override;
    o::object_ptr visit_do_expression(ast::do_expression&) override;
    o::object_ptr visit_case_expression(ast::case_expression&) override;
    o::object_ptr visit_binary_expression(ast::binary_expression&) override;
    o::object_ptr visit_call_expression(ast::call_expression&) override;
    o::object_ptr visit_function_expression(ast::function_expression&) override;
    o::object_ptr visit_let_expression(ast::let_expression&) override;
    o::object_ptr visit_number(ast::number&) override;
    o::object_ptr visit_string(ast::string&) override;
    o::object_ptr visit_identifier(ast::identifier&) override;
    o::object_ptr visit_unit(ast::unit&) override;

  private:
    [[nodiscard]] env& current_env() noexcept;

    [[nodiscard]] bool had_error() const noexcept;

    const char* _source = nullptr;
    std::vector<diagnostic::diagnostic> _diagnostics;
    std::stack<env> _scopes;
};

}
