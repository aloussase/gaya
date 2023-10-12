#pragma once

#include <robin_hood.h>

#include <ast_visitor.hpp>

namespace gaya
{

/**
 * Pass for resolving unbound references to top level definitions.
 */
class Resolver final : public ast::ast_visitor
{
public:
    using scope = robin_hood::unordered_set<eval::key>;
    explicit Resolver(std::vector<scope>);

    void resolve(ast::node_ptr);

    [[nodiscard]] std::vector<diagnostic::diagnostic>
    diagnostics() const noexcept;

    ResultType visit_program(ast::program&) override;
    ResultType visit_declaration_stmt(ast::declaration_stmt&) override;
    ResultType visit_expression_stmt(ast::expression_stmt&) override;
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
    void begin_scope() noexcept;
    void end_scope() noexcept;

    void assign_scope(ast::identifier&) noexcept;

    std::vector<diagnostic::diagnostic> _diagnostics;
    std::vector<scope> _scopes;
};

}
