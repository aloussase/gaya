#pragma once

#include <array>
#include <filesystem>

#include <ast_visitor.hpp>
#include <diagnostic.hpp>
#include <env.hpp>
#include <object.hpp>
#include <parser.hpp>
#include <span.hpp>

namespace gaya::eval
{

namespace fs = std::filesystem;

class interpreter final : public ast::ast_visitor
{
public:
    interpreter(
        char** command_line_arguments,
        const uint32_t command_line_argument_count) noexcept;

    [[nodiscard]] std::optional<ResultType>
    eval(const std::string& filename, const char* source) noexcept;

    [[nodiscard]] std::optional<ResultType>
    eval(const std::string& filename, ast::node_ptr ast) noexcept;

    [[nodiscard]] std::vector<diagnostic::diagnostic>
    diagnostics() const noexcept;

    /// Return the name of the file currently being interpreted.
    [[nodiscard]] const std::string& current_filename() const noexcept;

    /**
     * Report the interpreter's diagnostics.
     */
    void report_diagnostics() noexcept;

    /// Remove all diagnostics from this interpreter.
    void clear_diagnostics() noexcept;

    /// Get the interpreter's environment.
    [[nodiscard]] env& environment() noexcept;

    /// Define a new symbol in the current scope.
    void define(key, ResultType) noexcept;

    /// Begin a new scope.
    void begin_scope(env new_scope) noexcept;

    /// End a previous scope.
    void end_scope() noexcept;

    /// Add an interpreter error.
    void interp_error(span, const std::string& msg);

    /// Add an interpreter hint.
    void interp_hint(span, const std::string& hint);

    /// Whether the interpreter had an error.
    [[nodiscard]] bool had_error() const noexcept;

    /**
     * @return The stack of scopes of this interpreter.
     */
    [[nodiscard]] const std::vector<env>& scopes() const noexcept;

    /**
     * @return The parser that the interpreter uses.
     */
    [[nodiscard]] parser& get_parser() noexcept;

    /**
     * Execute a match pattern. Exposed for object::function::call.
     */
    [[nodiscard]] bool match_pattern(
        ResultType&,
        const ast::match_pattern&,
        std::function<key(const std::string&)> to_key = &key::local) noexcept;

    /**
     * Get a previously declared type.
     */
    [[nodiscard]] std::optional<types::Type>
    get_type(const std::string&) const noexcept;

    /* Visitor pattern */

    ResultType visit_program(ast::program&) override;
    ResultType visit_declaration_stmt(ast::declaration_stmt&) override;
    ResultType visit_expression_stmt(ast::expression_stmt&) override;

    ResultType assign_to_call_expression(
        const ast::call_expression&,
        object::object) noexcept;
    ResultType assign_to_get_expression(
        const ast::get_expression&,
        object::object) noexcept;
    ResultType
    assign_to_identifier(const ast::identifier&, object::object) noexcept;
    ResultType visit_assignment_stmt(ast::assignment_stmt&) override;

    ResultType visit_while_stmt(ast::while_stmt&) override;
    ResultType visit_for_in_stmt(ast::for_in_stmt&) override;
    ResultType visit_include_stmt(ast::include_stmt&) override;
    ResultType visit_type_declaration(ast::TypeDeclaration&) override;
    ResultType visit_struct_declaration(ast::StructDeclaration&) override;

    ResultType visit_do_expression(ast::do_expression&) override;
    ResultType visit_case_expression(ast::case_expression&) override;
    ResultType visit_match_expression(ast::match_expression&) override;
    ResultType visit_lnot_expression(ast::lnot_expression&) override;
    ResultType visit_not_expression(ast::not_expression&) override;
    ResultType visit_perform_expression(ast::perform_expression&) override;
    ResultType visit_binary_expression(ast::binary_expression&) override;
    ResultType visit_get_expression(ast::get_expression&) override;
    ResultType visit_call_expression(ast::call_expression&) override;
    ResultType visit_function_expression(ast::function_expression&) override;
    ResultType visit_let_expression(ast::let_expression&) override;
    ResultType visit_array(ast::array&) override;
    ResultType visit_dictionary(ast::dictionary&) override;
    ResultType visit_number(ast::number&) override;
    ResultType visit_string(ast::string&) override;
    ResultType visit_identifier(ast::identifier&) override;
    ResultType visit_unit(ast::unit&) override;
    ResultType visit_placeholder(ast::placeholder&) override;

private:
    std::string _filename;
    parser _parser;
    std::vector<diagnostic::diagnostic> _diagnostics;
    std::vector<env> _scopes;

    int _placeholders_in_use      = 0;
    bool _had_unused_placeholders = false;

    std::unordered_map<std::string, types::Type> _declared_types;

    char** _command_line_arguments;
    const uint32_t _command_line_argument_count;
};

}
