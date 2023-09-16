#pragma once

#include <stack>

#include <ast_visitor.hpp>
#include <diagnostic.hpp>
#include <env.hpp>
#include <object.hpp>
#include <span.hpp>

namespace gaya::eval
{

class interpreter final : public ast::ast_visitor
{
public:
    interpreter(const std::string& filename, const char* source);

    [[nodiscard]] object::maybe_object eval() noexcept;

    /// This one is useful for REPLs.
    [[nodiscard]] object::maybe_object eval(env, ast::node_ptr) noexcept;

    [[nodiscard]] std::vector<diagnostic::diagnostic>
    diagnostics() const noexcept;

    /// Return the name of the file currently being interpreted.
    [[nodiscard]] const std::string& current_filename() const noexcept;

    /// Remove all diagnostics from this interpreter.
    void clear_diagnostics() noexcept;

    /// Get the interpreter's environment.
    [[nodiscard]] const env& get_env() const noexcept;

    /// Define a new symbol in the current scope.
    void define(const key&, object::object) noexcept;

    /// Begin a new scope.
    void begin_scope(env new_scope) noexcept;

    /// End a previous scope.
    void end_scope() noexcept;

    /// Add an interpreter error.
    void interp_error(span, const std::string& msg);

    /// Add an interpreter hint.
    void interp_hint(span, const std::string& hint);

    /// Evaluate the given file in the context of this interpreter.
    [[nodiscard]] bool loadfile(const std::string&) noexcept;

    /// Whether the interpreter had an error.
    [[nodiscard]] bool had_error() const noexcept;

    /* Visitor pattern */

    object::maybe_object visit_program(ast::program&) override;
    object::maybe_object
    visit_declaration_stmt(ast::declaration_stmt&) override;
    object::maybe_object visit_expression_stmt(ast::expression_stmt&) override;
    object::maybe_object visit_assignment_stmt(ast::assignment_stmt&) override;
    object::maybe_object visit_while_stmt(ast::while_stmt&) override;
    object::maybe_object visit_do_expression(ast::do_expression&) override;
    object::maybe_object visit_case_expression(ast::case_expression&) override;
    object::maybe_object
    visit_unary_expression(ast::unary_expression&) override;
    object::maybe_object
    visit_binary_expression(ast::binary_expression&) override;
    object::maybe_object visit_call_expression(ast::call_expression&) override;
    object::maybe_object
    visit_function_expression(ast::function_expression&) override;
    object::maybe_object visit_let_expression(ast::let_expression&) override;
    object::maybe_object visit_array(ast::array&) override;
    object::maybe_object visit_number(ast::number&) override;
    object::maybe_object visit_string(ast::string&) override;
    object::maybe_object visit_identifier(ast::identifier&) override;
    object::maybe_object visit_unit(ast::unit&) override;
    object::maybe_object visit_placeholder(ast::placeholder&) override;

private:
    [[nodiscard]] env& current_env() noexcept;

    std::string _filename;
    const char* _source = nullptr;
    std::vector<diagnostic::diagnostic> _diagnostics;
    std::stack<env> _scopes;
};

}
