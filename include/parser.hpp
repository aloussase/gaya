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

private:
    std::vector<ast::stmt_ptr> stmts() noexcept;

    [[nodiscard]] ast::stmt_ptr toplevel_stmt() noexcept;
    [[nodiscard]] ast::stmt_ptr declaration_stmt(token identifier);

    [[nodiscard]] ast::stmt_ptr local_stmt(token token) noexcept;
    [[nodiscard]] ast::stmt_ptr assignment_stmt(token identifier) noexcept;
    [[nodiscard]] ast::stmt_ptr while_stmt(token while_) noexcept;
    [[nodiscard]] ast::stmt_ptr expression_stmt(token discard);
    [[nodiscard]] ast::stmt_ptr include_stmt(token include) noexcept;

    [[nodiscard]] ast::expression_ptr expression(token);
    [[nodiscard]] ast::expression_ptr let_expression(token let);
    [[nodiscard]] ast::expression_ptr case_expression(token cases);
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

    [[nodiscard]] ast::expression_ptr call_expression(token token);

    [[nodiscard]] ast::expression_ptr primary_expression(token);
    [[nodiscard]] ast::expression_ptr function_expression(token lcurly);
    [[nodiscard]] ast::expression_ptr string(token) noexcept;
    [[nodiscard]] ast::expression_ptr array(token) noexcept;
    [[nodiscard]] ast::expression_ptr
        dictionary(token, ast::expression_ptr) noexcept;

    void parser_error(span, const std::string&);
    void parser_hint(span, const std::string&);

    [[nodiscard]] bool match(token_type) noexcept;
    [[nodiscard]] bool match(std::optional<token>, token_type) noexcept;
    [[nodiscard]] bool is_local_stmt(token);

    /* Resolving identifier locations */
    void begin_scope() noexcept;
    void end_scope() noexcept;

    void define(ast::identifier&) noexcept;
    void define(eval::key) noexcept;

    [[nodiscard]] bool assign_scope(std::shared_ptr<ast::identifier>&) noexcept;
    [[nodiscard]] bool assign_scope(std::unique_ptr<ast::identifier>&) noexcept;

    [[nodiscard]] bool
    is_valid_assignment_target(std::unique_ptr<ast::identifier>&) noexcept;

    lexer _lexer;
    std::vector<diagnostic::diagnostic> _diagnostics;

    using scope = robin_hood::unordered_set<eval::key>;
    std::vector<scope> _scopes;

    std::string _filename;

    std::unordered_set<std::string> _included_files;
};

}
