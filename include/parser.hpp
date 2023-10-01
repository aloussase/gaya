#pragma once

#include <exception>
#include <unordered_set>
#include <vector>

#include <robin_hood.h>

#include <ast.hpp>
#include <diagnostic.hpp>
#include <lexer.hpp>

namespace gaya
{

class parser
{
public:
    parser();

    [[nodiscard]] ast::node_ptr
    parse(const std::string& filename, const char* source) noexcept;

    [[nodiscard]] std::vector<diagnostic::diagnostic>& diagnostics() noexcept;

    /**
     * Parse an expression.
     */
    [[nodiscard]] ast::node_ptr
    parse_expression(const std::string& filename, const char*) noexcept;

    /**
     * Parse a statement.
     */
    [[nodiscard]] ast::node_ptr
    parse_statement(const std::string& filename, const char*) noexcept;

    /**
     * Print the parser's diagnostics.
     */
    void report_diagnostics() const noexcept;

    /**
     * Clear the parser's diagnostics.
     */
    void clear_diagnostics() noexcept;

    /**
     * Return whether there was a parsing error.
     */
    bool had_error() const noexcept;

private:
    std::vector<ast::stmt_ptr> stmts() noexcept;

    [[nodiscard]] ast::stmt_ptr toplevel_stmt() noexcept;
    [[nodiscard]] ast::stmt_ptr declaration_stmt(token identifier);

    [[nodiscard]] ast::stmt_ptr local_stmt(token token) noexcept;
    [[nodiscard]] ast::stmt_ptr assignment_stmt(token) noexcept;
    [[nodiscard]] ast::stmt_ptr while_stmt(token while_) noexcept;
    [[nodiscard]] ast::stmt_ptr for_in_stmt(token for_) noexcept;
    [[nodiscard]] ast::stmt_ptr expression_stmt(token discard);
    [[nodiscard]] ast::stmt_ptr include_stmt(token include) noexcept;
    [[nodiscard]] ast::stmt_ptr type_declaration(token type) noexcept;
    [[nodiscard]] ast::stmt_ptr foreign_declaration(token foreign) noexcept;

    [[nodiscard]] std::optional<types::Type> type_id() noexcept;

    [[nodiscard]] std::vector<ast::StructField> struct_fields() noexcept;
    [[nodiscard]] ast::stmt_ptr struct_declaration(token struct_) noexcept;

    [[nodiscard]] ast::expression_ptr expression(token);
    [[nodiscard]] ast::expression_ptr let_expression(token let);
    [[nodiscard]] ast::expression_ptr case_expression(token cases);
    [[nodiscard]] std::optional<ast::match_pattern> match_pattern(
        const token&,
        bool define_matched_identifier                      = true,
        std::function<eval::key(const std::string&)> to_key = &eval::key::local,
        std::shared_ptr<ast::identifier>                    = nullptr) noexcept;
    [[nodiscard]] ast::expression_ptr match_expression(token target);
    [[nodiscard]] ast::expression_ptr do_expression(token token);

    [[nodiscard]] ast::expression_ptr logical_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr comparison_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr pipe_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr bitwise_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr term_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr factor_expression(token) noexcept;

    [[nodiscard]] ast::expression_ptr unary_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr lnot_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr not_expression(token) noexcept;
    [[nodiscard]] ast::expression_ptr perform_expression(token) noexcept;

    [[nodiscard]] std::shared_ptr<ast::function_expression>
    finish_trailing_function(span, token lparen) noexcept;
    [[nodiscard]] std::shared_ptr<ast::get_expression>
    finish_get_expression(span, ast::expression_ptr target) noexcept;
    [[nodiscard]] std::shared_ptr<ast::call_expression>
    finish_call_expression(span, ast::expression_ptr target) noexcept;
    [[nodiscard]] ast::expression_ptr call_expression(token token);

    [[nodiscard]] ast::expression_ptr primary_expression(token);
    [[nodiscard]] ast::expression_ptr function_expression(token lcurly);
    [[nodiscard]] ast::expression_ptr array(token) noexcept;
    [[nodiscard]] ast::expression_ptr
        dictionary(token, ast::expression_ptr) noexcept;

    [[nodiscard]] bool is_hex_digit(char c) const noexcept;
    [[nodiscard]] int to_hex_value(char c) const noexcept;
    [[nodiscard]] int
    hex_string_to_int(const std::string&, size_t&) const noexcept;
    [[nodiscard]] ast::expression_ptr string(token) noexcept;
    [[nodiscard]] ast::expression_ptr number(token) noexcept;

    void parser_error(span, const std::string&);
    void parser_hint(span, const std::string&);

    [[nodiscard]] bool match(token_type) noexcept;
    [[nodiscard]] bool match(std::optional<token>, token_type) noexcept;

    /* Resolving identifier locations */
    void begin_scope() noexcept;
    void end_scope() noexcept;

    void define(ast::identifier&) noexcept;
    void define(eval::key) noexcept;

    [[nodiscard]] bool assign_scope(std::shared_ptr<ast::identifier>&) noexcept;
    [[nodiscard]] bool assign_scope(std::unique_ptr<ast::identifier>&) noexcept;

    [[nodiscard]] bool is_valid_assignment_target(ast::identifier&) noexcept;

    lexer _lexer;
    std::vector<diagnostic::diagnostic> _diagnostics;

    using scope = robin_hood::unordered_set<eval::key>;
    std::vector<scope> _scopes;

    std::string _filename;

    std::unordered_set<std::string> _included_files;

    using type_declaration_ptr = std::shared_ptr<ast::TypeDeclaration>;
    std::unordered_map<std::string, type_declaration_ptr> _type_declarations;
};

}
