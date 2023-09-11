#pragma once

#include <memory>
#include <vector>

#include <lexer.hpp>
#include <span.hpp>

namespace gaya::eval
{
class interpreter;
}

namespace gaya::eval::object
{

struct object;
using object_ptr = std::shared_ptr<object>;

}

namespace gaya::ast
{

struct ast_node;
struct stmt;
struct expression;
struct identifier;

using node_ptr       = std::unique_ptr<ast_node>;
using stmt_ptr       = std::unique_ptr<stmt>;
using expression_ptr = std::unique_ptr<expression>;

class ast_visitor;

struct ast_node
{
    virtual ~ast_node() {};
    virtual std::string to_string() const noexcept              = 0;
    virtual gaya::eval::object::object_ptr accept(ast_visitor&) = 0;
};

struct program final : public ast_node
{
    std::vector<stmt_ptr> stmts;

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;
};

/* Statements */

struct stmt : public ast_node
{
    virtual ~stmt() { }
};

struct declaration_stmt final : public stmt
{
    declaration_stmt(std::unique_ptr<identifier> i, expression_ptr e)
        : _identifier { std::move(i) }
        , expression { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    std::unique_ptr<identifier> _identifier;
    expression_ptr expression;
};

struct expression_stmt final : public stmt
{
    expression_stmt(expression_ptr e)
        : expr { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    expression_ptr expr;
};

/* Expressions */

struct expression : public ast_node
{
    virtual ~expression() { }
};

struct do_expression final : public expression
{
    do_expression(span s, std::vector<node_ptr>&& b)
        : span_ { s }
        , body { std::move(b) }
    {
    }

    std::string to_string() const noexcept override;
    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    // All but the last node in a do block body must be stmts.
    // The value of a do block is the value of its last expression.
    span span_;
    std::vector<node_ptr> body;
};

struct case_branch final
{
    case_branch(expression_ptr c, expression_ptr b)
        : condition { std::move(c) }
        , body { std::move(b) }
    {
    }

    [[nodiscard]] std::string to_string() const noexcept;

    expression_ptr condition;
    expression_ptr body;
};

struct case_expression final : public expression
{
    case_expression(
        span s,
        std::vector<case_branch> bs,
        expression_ptr o = nullptr)
        : span_ { s }
        , branches { std::move(bs) }
        , otherwise { std::move(o) }
    {
    }

    std::string to_string() const noexcept override;
    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span span_;
    std::vector<case_branch> branches;
    expression_ptr otherwise;
};

struct call_expression final : public expression
{
    call_expression(span s, expression_ptr t, std::vector<expression_ptr>&& a)
        : span_ { s }
        , target { std::move(t) }
        , args { std::move(a) }
    {
    }

    std::string to_string() const noexcept override;
    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span span_;
    expression_ptr target;
    std::vector<expression_ptr> args;
};

struct function_expression final : public expression
{
    function_expression(span s, std::vector<identifier>&& p, expression_ptr b)
        : _span { s }
        , params { std::move(p) }
        , body { std::move(b) }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span _span;
    std::vector<identifier> params;
    std::shared_ptr<expression> body;
};

struct let_expression final : public expression
{
    let_expression(
        std::unique_ptr<identifier> i, //
        expression_ptr b,
        expression_ptr e)
        : ident { std::move(i) }
        , binding { std::move(b) }
        , expr { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    std::unique_ptr<identifier> ident;
    expression_ptr binding;
    expression_ptr expr;
};

/* Binary expressions */

struct binary_expression : public expression
{
    virtual ~binary_expression() { }
    virtual gaya::eval::object::object_ptr execute(eval::interpreter&) = 0;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;
};

struct cmp_expression final : public binary_expression
{
    cmp_expression(expression_ptr l, token o, expression_ptr r)
        : lhs { std::move(l) }
        , op { o }
        , rhs { std::move(r) }
    {
    }

    gaya::eval::object::object_ptr execute(eval::interpreter&) override;
    std::string to_string() const noexcept override;

    expression_ptr lhs;
    token op;
    expression_ptr rhs;
};

/* Primary expressions */

struct number final : public expression
{
    number(span s, double v)
        : _span { s }
        , value { v }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span _span;
    double value;
};

struct string final : public expression
{
    string(span s, const std::string& v)
        : _span { s }
        , value { v }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span _span;
    std::string value;
};

struct identifier final : public expression
{
    identifier(span s, const std::string& v)
        : _span { s }
        , value { v }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span _span;
    std::string value;
};

struct unit final : public expression
{
    unit(span s)
        : _span { s }
    {
    }

    std::string to_string() const noexcept override;

    gaya::eval::object::object_ptr accept(ast_visitor&) override;

    span _span;
};

}
