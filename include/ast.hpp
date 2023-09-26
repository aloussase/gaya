#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <env.hpp>
#include <lexer.hpp>
#include <object.hpp>
#include <span.hpp>
#include <types.hpp>

namespace gaya::eval
{
class interpreter;
}

namespace gaya::ast
{

using object = gaya::eval::object::object;

struct ast_node;
struct stmt;
struct expression;
struct identifier;

using node_ptr       = std::shared_ptr<ast_node>;
using stmt_ptr       = std::shared_ptr<stmt>;
using expression_ptr = std::shared_ptr<expression>;

template <typename T, typename... Ts>
std::shared_ptr<T> make_node(Ts&&... args)
{
    return std::make_shared<T>(std::forward<Ts>(args)...);
}

class ast_visitor;

struct ast_node
{
    virtual ~ast_node() {};
    virtual std::string to_string() const noexcept = 0;
    virtual object accept(ast_visitor&)            = 0;
};

struct program final : public ast_node
{
    std::vector<stmt_ptr> stmts;

    std::string to_string() const noexcept override;

    object accept(ast_visitor&) override;
};

/* Statements */

struct stmt : public ast_node
{
    virtual ~stmt() { }
};

struct declaration_stmt final : public stmt
{
    declaration_stmt(std::unique_ptr<identifier> i, expression_ptr e)
        : ident { std::move(i) }
        , expr { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    std::unique_ptr<identifier> ident;
    expression_ptr expr;
};

struct expression_stmt final : public stmt
{
    expression_stmt(expression_ptr e)
        : expr { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    expression_ptr expr;
};

struct assignment_stmt final : public stmt
{
    assignment_stmt(std::unique_ptr<identifier> i, expression_ptr e)
        : ident { std::move(i) }
        , expr { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    std::unique_ptr<identifier> ident;
    expression_ptr expr;
};

struct while_stmt final : public stmt
{
    while_stmt(
        span s,
        expression_ptr e,
        std::vector<stmt_ptr>&& b,
        stmt_ptr c = nullptr)
        : span_ { s }
        , condition { std::move(e) }
        , body { std::move(b) }
        , continuation { c }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    expression_ptr condition;
    std::vector<stmt_ptr> body;
    stmt_ptr continuation;
};

struct for_in_stmt final : public stmt
{
    for_in_stmt(
        span s,
        std::shared_ptr<identifier> i,
        expression_ptr seq,
        std::vector<stmt_ptr> b)
        : span_ { s }
        , ident { i }
        , sequence { seq }
        , body { b }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    std::shared_ptr<identifier> ident;
    expression_ptr sequence;
    std::vector<stmt_ptr> body;
};

struct include_stmt final : public stmt
{
    include_stmt(span s, const std::string& f, ast::node_ptr p)
        : span_ { s }
        , file_path { f }
        , parsed_file { p }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    std::filesystem::path file_path;
    ast::node_ptr parsed_file;
};

struct TypeDeclaration final : public stmt
{
    TypeDeclaration(
        span s,
        const std::string& d_t,
        types::Type u_t,
        expression_ptr c)
        : span_ { s }
        , declared_type { d_t }
        , underlying_type { u_t }
        , constraint { c }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    std::string declared_type;
    types::Type underlying_type;
    expression_ptr constraint;
};

struct ForeignDeclaration final : public stmt
{
    ForeignDeclaration(
        const std::string& lname,
        const std::string& fname,
        types::ForeignType rt,
        std::vector<types::ForeignType> at)
        : libname { lname }
        , funcname { fname }
        , return_type { rt }
        , argument_types { std::move(at) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    std::string libname;
    std::string funcname;
    types::ForeignType return_type;
    std::vector<types::ForeignType> argument_types;
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
    object accept(ast_visitor&) override;

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
    object accept(ast_visitor&) override;

    span span_;
    std::vector<case_branch> branches;
    expression_ptr otherwise;
};

struct match_pattern final
{
    enum class kind { wildcard, capture, expr, array_pattern };

    match_pattern(
        kind k,
        expression_ptr v                = nullptr,
        std::shared_ptr<identifier> a_p = nullptr)
        : kind { k }
        , value { v }
        , as_pattern { a_p }
    {
    }

    match_pattern(
        kind k,
        std::vector<match_pattern> v,
        std::shared_ptr<identifier> a_p = nullptr)
        : kind { k }
        , value { std::move(v) }
        , as_pattern { a_p }
    {
    }

    kind kind;
    std::variant<expression_ptr, std::vector<match_pattern>> value;
    std::shared_ptr<identifier> as_pattern = nullptr;
};

struct match_branch final
{
    match_branch(match_pattern p, expression_ptr b, expression_ptr c = nullptr)
        : pattern { p }
        , body { b }
        , condition { c }
    {
    }

    match_pattern pattern;
    expression_ptr body;
    expression_ptr condition = nullptr;
};

struct match_expression final : expression
{
    match_expression(
        span s,
        expression_ptr t,
        std::vector<match_branch> b,
        expression_ptr o = nullptr)
        : span_ { s }
        , target { t }
        , branches { b }
        , otherwise { o }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    expression_ptr target;
    std::vector<match_branch> branches;
    expression_ptr otherwise = nullptr;
};

struct call_expression final : public expression
{
    call_expression(span s, expression_ptr t, std::vector<expression_ptr>&& a)
        : span_ { s }
        , target { std::move(t) }
        , args { std::move(a) }
        , oargs(args.size(), gaya::eval::object::invalid)
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    expression_ptr target;
    std::vector<expression_ptr> args;
    std::vector<gaya::eval::object::object> oargs;
};

struct function_param final
{
    function_param(match_pattern p, expression_ptr dv = nullptr)
        : function_param { p, types::Type { types::TypeKind::Any }, dv }
    {
    }

    function_param(
        match_pattern p,
        types::Type t     = types::Type { types::TypeKind::Any },
        expression_ptr dv = nullptr)
        : pattern { std::move(p) }
        , type { t }
        , default_value { dv }
    {
    }

    match_pattern pattern;
    types::Type type;
    expression_ptr default_value = nullptr;
};

struct function_expression final : public expression
{
    function_expression(
        span s,
        std::vector<function_param>&& p,
        expression_ptr b)
        : _span { s }
        , params { std::move(p) }
        , body { std::move(b) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span _span;
    std::vector<function_param> params;
    std::shared_ptr<expression> body;
};

struct let_binding
{
    let_binding(span s, match_pattern p, expression_ptr b)
        : span_ { s }
        , pattern { std::move(p) }
        , value { std::move(b) }

    {
    }

    span span_;
    match_pattern pattern;
    expression_ptr value;
};

struct let_expression final : public expression
{
    let_expression(std::vector<let_binding> b, expression_ptr e)
        : bindings { std::move(b) }
        , expr { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    std::vector<let_binding> bindings;
    expression_ptr expr;
};

/* Binary expressions */

struct binary_expression final : public expression
{
    binary_expression(expression_ptr l, token o, expression_ptr r)
        : lhs { std::move(l) }
        , op { o }
        , rhs { std::move(r) }
    {
    }

    object accept(ast_visitor&) override;
    std::string to_string() const noexcept override;

    expression_ptr lhs;
    token op;
    expression_ptr rhs;
};

/* Unary expressions */

struct lnot_expression final : public expression
{
    lnot_expression(token o, expression_ptr oper)
        : op { o }
        , operand { oper }
    {
    }

    object accept(ast_visitor&) override;
    std::string to_string() const noexcept override;

    token op;
    expression_ptr operand;
};

struct not_expression final : public expression
{
    not_expression(token o, expression_ptr oper)
        : op { o }
        , operand { std::move(oper) }
    {
    }

    object accept(ast_visitor&) override;
    std::string to_string() const noexcept override;

    token op;
    expression_ptr operand;
};

struct perform_expression final : public expression
{
    perform_expression(token o, stmt_ptr s)
        : op { o }
        , stmt { std::move(s) }
    {
    }

    object accept(ast_visitor&) override;
    std::string to_string() const noexcept override;

    token op;
    stmt_ptr stmt;
};

/* Primary expressions */

struct array final : public expression
{
    array(span s, std::vector<expression_ptr> e)
        : span_ { s }
        , elems { std::move(e) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    std::vector<expression_ptr> elems;
};

struct dictionary final : public expression
{
    dictionary(
        span s,
        std::vector<expression_ptr> k,
        std::vector<expression_ptr> v)
        : span_ { s }
        , keys { std::move(k) }
        , values { std::move(v) }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
    std::vector<expression_ptr> keys;
    std::vector<expression_ptr> values;
};

struct number final : public expression
{
    number(span s, double v)
        : _span { s }
        , value { v }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

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
    object accept(ast_visitor&) override;

    span _span;
    std::string value;
};

struct identifier final : public expression
{
    identifier(span s, const std::string& v)
        : _span { s }
        , value { v }
        , key { value }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span _span;
    std::string value;
    gaya::eval::key key;
    size_t depth = 0;
};

struct unit final : public expression
{
    unit(span s)
        : _span { s }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span _span;
};

struct placeholder final : public expression
{
    placeholder(span s)
        : span_ { s }
    {
    }

    std::string to_string() const noexcept override;
    object accept(ast_visitor&) override;

    span span_;
};

}
