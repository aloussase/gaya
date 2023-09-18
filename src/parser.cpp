#include <cassert>
#include <iostream>
#include <memory>

#include <fmt/core.h>

#include <ast.hpp>
#include <file_reader.hpp>
#include <parser.hpp>

namespace gaya
{

parser::parser()
    : _lexer { nullptr }
{
    using namespace std::string_literals;

    begin_scope();

    /* Define builtins */

    define("typeof"s);
    define("assert"s);
    define("tostring"s);
    define("tosequence"s);
    define("issequence"s);
    define("io.println"s);
    define("string.length"s);
    define("string.concat"s);
    define("array.length"s);
    define("array.concat"s);
    define("array.push"s);
    define("seq.next"s);
    define("seq.make"s);
}

std::vector<diagnostic::diagnostic>& parser::diagnostics() noexcept
{
    return _diagnostics;
}

void parser::parser_error(span s, const std::string& message)
{
    _diagnostics
        .emplace_back(s, message, diagnostic::severity::error, _filename);
}

void parser::parser_hint(span s, const std::string& message)
{
    _diagnostics
        .emplace_back(s, message, diagnostic::severity::hint, _filename);
}

bool parser::match(std::optional<token> t, token_type tt) const noexcept
{
    return t && t->type == tt;
}

bool parser::is_local_stmt(token token)
{
    if (token.type == token_type::discard) return true;
    if (token.type == token_type::while_) return true;
    if (token.type != token_type::identifier) return false;

    auto lookahead = _lexer.next_token();
    auto ret       = false;

    if (match(lookahead, token_type::back_arrow))
    {
        ret = true;
    }

    _lexer.push_back(lookahead.value());
    return ret;
}

void parser::begin_scope() noexcept
{
    _scopes.push_back({});
}

void parser::end_scope() noexcept
{
    assert(_scopes.size() > 0);
    _scopes.pop_back();
}

void parser::define(eval::key key) noexcept
{
    assert(_scopes.size() > 0);
    _scopes.back().insert(key);
}

void parser::define(ast::identifier& ident) noexcept
{
    define(ident.key);
}

bool parser::assign_scope(std::shared_ptr<ast::identifier>& ident) noexcept
{
    assert(_scopes.size() > 0);

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (_scopes[i].contains(ident->key))
        {
            ident->depth = _scopes.size() - 1 - i;
            return true;
        }
    }
    return false;
}

bool parser::assign_scope(std::unique_ptr<ast::identifier>& ident) noexcept
{
    assert(_scopes.size() > 0);

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (_scopes[i].contains(ident->key))
        {
            ident->depth = _scopes.size() - 1 - i;
            return true;
        }
    }
    return false;
}

bool parser::is_valid_assignment_target(
    std::unique_ptr<ast::identifier>& ident) noexcept
{
    assert(_scopes.size() > 0);

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (auto it = _scopes[i].find(ident->key); it != _scopes[i].end())
        {
            return it->kind == eval::identifier_kind::local;
        }
    }
    return false;
}

ast::node_ptr
parser::parse(const std::string& filename, const char* source) noexcept
{
    /* Create a new lexer for the given source code. */
    _filename = filename;
    _lexer    = lexer { source };

    auto program   = ast::make_node<ast::program>();
    program->stmts = stmts();

    /* Merge any lexer diagnostics. */
    for (const auto& diag : _lexer.diagnostics())
    {
        _diagnostics.insert(_diagnostics.begin(), diag);
    }

    return program;
}

std::vector<ast::stmt_ptr> parser::stmts() noexcept
{
    std::vector<ast::stmt_ptr> _stmts;

    for (;;)
    {
        if (auto stmt = toplevel_stmt(); stmt)
        {
            _stmts.push_back(std::move(stmt));
        }
        else
        {
            break;
        }
    }

    return _stmts;
}

ast::stmt_ptr parser::toplevel_stmt() noexcept
{
    auto token = _lexer.next_token();
    if (!token) return nullptr;

    switch (token->type)
    {
    case token_type::identifier: return declaration_stmt(token.value());
    case token_type::discard: return expression_stmt(token.value());
    default:
        parser_error(token->span, "Invalid start of top-level statement");
        parser_hint(
            token->span,
            "Only definitions and discards are valid top-level statements");
        return nullptr;
    }
}

ast::stmt_ptr parser::local_stmt(token token) noexcept
{
    switch (token.type)
    {
    case token_type::discard: return expression_stmt(token);
    case token_type::identifier: return assignment_stmt(token);
    case token_type::while_: return while_stmt(token);
    default:
        parser_error(token.span, "Invalid start of local statement");
        parser_hint(
            token.span,
            "Only discard, assignments and while loops are valid local "
            "statements");
        return nullptr;
    }
}

ast::stmt_ptr parser::assignment_stmt(token identifier) noexcept
{
    if (auto back_arrow = _lexer.next_token();
        !match(back_arrow, token_type::back_arrow))
    {
        parser_error(
            identifier.span,
            "Expected '<-' after identifier in assignment statement");
        return nullptr;
    }

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            identifier.span,
            "Expected expression after '<-' in assignment statement");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    auto ident = std::make_unique<ast::identifier>(
        identifier.span,
        identifier.span.to_string());

    if (!is_valid_assignment_target(ident))
    {
        parser_error(
            identifier.span,
            fmt::format(
                "Invalid assignment target: {}",
                identifier.span.to_string()));
        parser_hint(
            identifier.span,
            "Only variables introduced with let are valid assignment targets");
        return nullptr;
    }

    if (!assign_scope(ident))
    {
        parser_error(
            identifier.span,
            fmt::format(
                "Undeclared identifier in assignment: {}",
                identifier.span.to_string()));
        return nullptr;
    }

    return ast::make_node<ast::assignment_stmt>(std::move(ident), expr);
}

ast::stmt_ptr parser::while_stmt(token while_) noexcept
{
    begin_scope();

    auto span  = while_.span;
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(span, "Expected a condition after 'while'");
        end_scope();
        return nullptr;
    }

    auto condition             = expression(token.value());
    ast::stmt_ptr continuation = nullptr;

    if (auto colon = _lexer.next_token(); match(colon, token_type::colon))
    {
        auto stmt_token = _lexer.next_token();
        if (!stmt_token || !is_local_stmt(stmt_token.value()))
        {
            parser_error(
                while_.span,
                "Expected a statement after ':' in while");
            end_scope();
            return nullptr;
        }

        continuation = local_stmt(stmt_token.value());
    }
    else if (colon)
    {
        _lexer.push_back(colon.value());
    }

    std::vector<ast::stmt_ptr> body;

    for (;;)
    {
        auto token = _lexer.next_token();
        if (!token) break;

        if (is_local_stmt(token.value()))
        {
            auto stmt = local_stmt(token.value());
            if (!stmt)
            {
                end_scope();
                return nullptr;
            }

            body.push_back(std::move(stmt));
        }
        else
        {
            _lexer.push_back(token.value());
            break;
        }
    }

    end_scope();

    if (auto end = _lexer.next_token(); !match(end, token_type::end))
    {
        parser_error(span, "Expected 'end' after while body");
        parser_hint(span, "You can only use local statements in while body");
        return nullptr;
    }

    return ast::make_node<ast::while_stmt>(
        span,
        condition,
        std::move(body),
        continuation);
}

ast::stmt_ptr parser::declaration_stmt(token identifier)
{
    auto colon_colon = _lexer.next_token();
    if (!colon_colon || colon_colon->type != token_type::colon_colon)
    {
        parser_error(identifier.span, "Expected a '::' after identifier");
        parser_hint(identifier.span, "Maybe you meant to use discard?");
        return nullptr;
    }

    auto ident = std::make_unique<ast::identifier>(
        identifier.span,
        identifier.span.to_string());

    /*
     * Here we are defining the identifier before evaluating its expression,
     * which means that we do support top level recursive declarations.
     */
    define(*ident);

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(colon_colon->span, "Expected an expression after '::'");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::declaration_stmt>(std::move(ident), expr);
}

ast::stmt_ptr parser::expression_stmt(token discard)
{
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(discard.span, "Expected an expression after discard");
        return nullptr;
    }

    assert(token && "parser::expression_stmt: expected token to have a value");
    auto expr = expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::expression_stmt>(expr);
}

ast::expression_ptr parser::expression(token token)
{
    /*
     * NOTE:
     *
     * We might want to move these to primary expression or allow them in
     * grouping expressions.
     */
    switch (token.type)
    {
    case token_type::do_: return do_expression(token);
    case token_type::cases: return case_expression(token);
    default: return logical_expression(token);
    }
}

ast::expression_ptr parser::function_expression(token lcurly)
{
    begin_scope();

    std::vector<ast::identifier> params;

    for (;;)
    {
        auto identifier = _lexer.next_token();
        if (!identifier) break;

        if (!match(identifier, token_type::identifier))
        {
            _lexer.push_back(identifier.value());
            break;
        }

        auto span             = identifier->span;
        ast::identifier ident = { span, span.to_string() };

        define(ident);

        params.push_back(std::move(ident));

        auto comma = _lexer.next_token();
        if (!comma) break;

        if (!match(comma, token_type::comma))
        {
            _lexer.push_back(comma.value());
            break;
        }
    }

    auto arrow = _lexer.next_token();
    if (!match(arrow, token_type::arrow))
    {
        parser_error(lcurly.span, "Expected '=>' after function params");
        end_scope();
        return nullptr;
    }

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(lcurly.span, "Expected a '}' after function body");
        end_scope();
        return nullptr;
    }

    auto expr = ([&]() -> ast::expression_ptr {
        if (match(token, token_type::rcurly))
        {
            return ast::make_node<ast::unit>(token->span);
        }
        else
        {
            auto expr = expression(token.value());
            if (auto rcurly = _lexer.next_token();
                !match(rcurly, token_type::rcurly))
            {
                parser_error(lcurly.span, "Expected a '}' after function body");
                return nullptr;
            }
            return expr;
        }
    })();

    end_scope();

    if (!expr) return nullptr;

    return ast::make_node<ast::function_expression>(
        lcurly.span,
        std::move(params),
        expr);
}

ast::expression_ptr parser::logical_expression(token token) noexcept
{
    auto lhs = comparison_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::or_:
        case token_type::and_:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = comparison_expression(t.value());
            if (!rhs) return nullptr;

            lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

/*
 * comparison_expression ::= pipe_expression '<' pipe_expression
 *                         | pipe_expression '>' pipe_expression
 *                         | pipe_expression '<=' pipe_expression
 *                         | pipe_expression '>=' pipe_expression
 *                         | pipe_expression '==' pipe_expression
 *                         | pipe_expression
 */
ast::expression_ptr parser::comparison_expression(token token) noexcept
{
    auto lhs = pipe_expression(token);
    if (!lhs) return nullptr;

    auto is_comparison_op = [](auto t) {
        switch (t.type)
        {
        case token_type::less_than:
        case token_type::less_than_eq:
        case token_type::greater_than:
        case token_type::greater_than_eq:
        case token_type::equal_equal:
        case token_type::not_equals: return true;
        default: return false;
        }
    };

    for (;;)
    {
        auto op = _lexer.next_token();
        if (!op) break;

        if (!is_comparison_op(op.value()))
        {
            _lexer.push_back(op.value());
            break;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                token.span,
                fmt::format(
                    "Expected an expression after {}",
                    token.span.to_string()));
            return nullptr;
        }

        auto rhs = pipe_expression(expr_token.value());

        if (!rhs) return nullptr;

        lhs = ast::make_node<ast::binary_expression>(lhs, op.value(), rhs);
    }

    return lhs;
}

/*
 * pipe_expression ::= term_expression '|>' term_expression
 */
ast::expression_ptr parser::pipe_expression(token token) noexcept
{
    auto lhs = term_expression(token);
    if (!lhs) return nullptr;

    for (;;)
    {
        auto pipe = _lexer.next_token();
        if (!match(pipe, token_type::pipe))
        {
            if (pipe)
            {
                _lexer.push_back(pipe.value());
            }
            break;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                token.span,
                "Expected an expression after pipe operator");
            return nullptr;
        }

        begin_scope();
        auto rhs = term_expression(expr_token.value());
        end_scope();

        if (!rhs) return nullptr;

        assert(lhs && rhs);

        lhs = ast::make_node<ast::binary_expression>(lhs, pipe.value(), rhs);
    }

    return lhs;
}

/**
 * term_expression ::= factor_expression '+' factor_expression
 *                   | factor_expression '-' factor_expression
 *                   | factor_expression
 */
ast::expression_ptr parser::term_expression(token token) noexcept
{
    auto lhs = factor_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::plus:
        case token_type::dash:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = factor_expression(t.value());
            if (!rhs) return nullptr;

            lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

/**
 * factor_expression ::= unary_expression '*' unary_expression
 *                     | unary_expression '/' unary_expression
 *                     | unary_expression
 */
ast::expression_ptr parser::factor_expression(token token) noexcept
{

    auto lhs = unary_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::star:
        case token_type::slash:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.span,
                    fmt::format(
                        "Expected an expression after {}",
                        op.span.to_string()));
                return nullptr;
            }

            auto rhs = unary_expression(t.value());
            if (!rhs) return nullptr;

            lhs = ast::make_node<ast::binary_expression>(lhs, op, rhs);

            break;
        }
        default:
        {
            _lexer.push_back(t.value());
            done = true;
            break;
        }
        }
    }

    return lhs;
}

/*
 * unary_expression ::= 'not' call_expression
 */
ast::expression_ptr parser::unary_expression(token op) noexcept
{
    switch (op.type)
    {
    case token_type::perform: return perform_expression(op);
    case token_type::not_: return not_expression(op);
    default: return call_expression(op);
    }
}

ast::expression_ptr parser::not_expression(token op) noexcept
{
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            op.span,
            fmt::format(
                "Expected expression after 'not'",
                op.span.to_string()));
        return nullptr;
    }

    auto expr = call_expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::not_expression>(op, expr);
}

ast::expression_ptr parser::perform_expression(token op) noexcept
{
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(op.span, "Expected a statement after 'perform'");
        return nullptr;
    }

    auto stmt = local_stmt(token.value());
    if (!stmt) return nullptr;

    return ast::make_node<ast::perform_expression>(op, stmt);
}

/*
 * call_expression ::= primary_expression '(' args_list ')'
 *                   | primary_expression
 *
 * args_list ::= expression
 *             | expression ',' args_list
 */
ast::expression_ptr parser::call_expression(token starttoken)
{
    auto assert_token = [](auto token) {
        assert(
            token && "parser::call_expression: expected token to have a value");
    };

    auto expr = primary_expression(starttoken);
    if (!expr) return nullptr;

    for (;;)
    {
        auto lparen = _lexer.next_token();
        if (!lparen) break;

        if (!match(lparen, token_type::lparen))
        {
            // Not a function call.
            assert_token(lparen);
            _lexer.push_back(lparen.value());
            break;
        }

        std::vector<ast::expression_ptr> args;

        for (;;)
        {
            auto expr_token = _lexer.next_token();
            if (!expr_token) break;

            if (match(expr_token, token_type::rparen))
            {
                assert_token(expr_token);
                _lexer.push_back(expr_token.value());
                break;
            }

            assert_token(expr_token);
            auto arg = expression(expr_token.value());
            if (!arg) return nullptr;

            args.push_back(std::move(arg));

            auto comma = _lexer.next_token();
            if (!comma) break;

            if (!match(comma, token_type::comma))
            {
                assert_token(comma);
                _lexer.push_back(comma.value());
                break;
            }
        }

        if (auto rparen = _lexer.next_token();
            !match(rparen, token_type::rparen))
        {
            parser_error(starttoken.span, "Missing ')' after function call");
            return nullptr;
        }

        expr = ast::make_node<ast::call_expression>(
            starttoken.span,
            expr,
            std::move(args));
    }

    return expr;
}

ast::expression_ptr parser::let_expression(token let)
{
    begin_scope();

    auto let_binding = [&](token ident) -> std::optional<ast::let_binding> {
        if (auto equal_sign = _lexer.next_token();
            !match(equal_sign, token_type::equal))
        {
            parser_error(
                ident.span,
                "Expected '=' after identifier in let expression");
            return std::nullopt;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(ident.span, "Expected an expression after '='");
            return std::nullopt;
        }

        auto value = expression(expr_token.value());
        if (!value) return std::nullopt;

        auto identifier = std::make_unique<ast::identifier>(
            ident.span,
            ident.span.to_string());

        /*
         * Since we are defining the identifier after evaluating its
         * value, that means we don't support recursive definitions
         */
        define(eval::key::local(identifier->value));

        return ast::let_binding { std::move(identifier), std::move(value) };
    };

    std::vector<ast::let_binding> bindings;
    for (;;)
    {
        auto ident = _lexer.next_token();
        if (!ident) break;

        if (!match(ident, token_type::identifier))
        {
            _lexer.push_back(ident.value());
            break;
        }

        auto binding = let_binding(ident.value());
        if (!binding) return nullptr;

        bindings.push_back(std::move(binding.value()));

        if (auto comma = _lexer.next_token(); !match(comma, token_type::comma))
        {
            if (comma)
            {
                _lexer.push_back(comma.value());
            }
            break;
        }
    }

    if (bindings.empty())
    {
        parser_error(
            let.span,
            "Expected at least one binding in let expression");
        end_scope();
        return nullptr;
    }

    if (auto in = _lexer.next_token(); !match(in, token_type::in))
    {
        parser_error(
            let.span,
            "Expected 'in' after expression in let expression");
        end_scope();
        return nullptr;
    }

    auto expr_token = _lexer.next_token();
    if (!expr_token)
    {
        parser_error(let.span, "Expected an expression after 'in'");
        end_scope();
        return nullptr;
    }

    auto expr = expression(expr_token.value());
    if (!expr)
    {
        end_scope();
        return nullptr;
    }

    end_scope();

    return ast::make_node<ast::let_expression>(std::move(bindings), expr);
}

ast::expression_ptr parser::case_expression(token cases)
{
    std::vector<ast::case_branch> branches;

    for (;;)
    {
        auto token = _lexer.next_token();
        if (!token) break;

        if (!match(token, token_type::given))
        {
            _lexer.push_back(token.value());
            break;
        }

        token = _lexer.next_token();
        if (!token) return nullptr;

        auto condition = expression(token.value());
        if (!condition) return nullptr;

        token = _lexer.next_token();
        if (!match(token, token_type::arrow))
        {
            parser_error(cases.span, "Expected a '=>' after condition in case");
            return nullptr;
        }

        token = _lexer.next_token();
        if (!token)
        {
            parser_error(
                cases.span,
                "Expected an expression after condition in case");
            return nullptr;
        }

        auto body = expression(token.value());

        branches.push_back(ast::case_branch {
            std::move(condition),
            std::move(body),
        });
    }

    auto token = _lexer.next_token();
    if (!token) return nullptr;

    ast::expression_ptr otherwise = nullptr;

    if (match(token, token_type::otherwise))
    {
        token = _lexer.next_token();
        if (!match(token, token_type::arrow))
        {
            parser_error(cases.span, "Expected '=>' after otherwise");
            return nullptr;
        }

        token = _lexer.next_token();
        if (!token)
        {
            parser_error(cases.span, "Expected and expression after otherwise");
            return nullptr;
        }

        otherwise = expression(token.value());
        if (!otherwise) return nullptr;
    }
    else
    {
        _lexer.push_back(token.value());
    }

    token = _lexer.next_token();
    if (!match(token, token_type::end))
    {
        parser_error(cases.span, "Expected 'end' after case");
        return nullptr;
    }

    return ast::make_node<ast::case_expression>(
        cases.span,
        std::move(branches),
        otherwise);
}

ast::expression_ptr parser::do_expression(token token)
{
    begin_scope();

    std::vector<ast::node_ptr> body;
    bool parsed_final_expression = false;

    for (;;)
    {
        auto tk = _lexer.next_token();
        if (!tk) break;

        if (match(tk, token_type::end))
        {
            _lexer.push_back(tk.value());
            break;
        }

        if (is_local_stmt(tk.value()))
        {
            auto stmt = local_stmt(tk.value());
            if (!stmt)
            {
                end_scope();
                return nullptr;
            }

            body.push_back(std::move(stmt));
        }
        else
        {
            auto expr = expression(tk.value());
            if (!expr)
            {
                end_scope();
                return nullptr;
            }

            body.push_back(std::move(expr));
            parsed_final_expression = true;
            break;
        }
    }

    if (!parsed_final_expression)
    {
        body.push_back(ast::make_node<ast::unit>(token.span));
    }

    end_scope();

    if (auto tk = _lexer.next_token(); !match(tk, token_type::end))
    {
        parser_error(
            token.span,
            "Expected 'end' after last expression in do block");
        parser_hint(
            token.span,
            "Check that you don't have leftover expressions in the do block");
        return nullptr;
    }

    return ast::make_node<ast::do_expression>(token.span, std::move(body));
}

/*
 * primary_expression ::= NUMBER
 *                      | IDENTIFIER
 *                      | STRING
 *                      | UNIT
 */
ast::expression_ptr parser::primary_expression(token token)
{
    switch (token.type)
    {
    case token_type::number:
    {
        return ast::make_node<ast::number>(
            token.span,
            std::stod(token.span.to_string()));
    }
    case token_type::string:
    {
        return string(token);
    }
    case token_type::identifier:
    {
        auto ident = ast::make_node<ast::identifier>(
            token.span,
            token.span.to_string());

        if (!assign_scope(ident))
        {
            parser_error(
                token.span,
                fmt::format(
                    "Undefined identifier '{}'",
                    token.span.to_string()));
            return nullptr;
        }

        return ident;
    }
    case token_type::unit:
    {
        return ast::make_node<ast::unit>(token.span);
    }
    case token_type::lcurly:
    {
        return function_expression(token);
    }
    case token_type::let:
    {
        return let_expression(token);
    }
    case token_type::lparen:
    {
        return array(token);
    }
    case token_type::underscore:
    {
        return ast::make_node<ast::placeholder>(token.span);
    }
    default:
        parser_error(token.span, "Invalid start of primary expression");
        return nullptr;
    }
}

ast::expression_ptr parser::string(token token) noexcept
{
    auto is_hex_digit = [](char c) {
        return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
            || (c >= '0' && c <= '9');
    };

    auto to_hex_value = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        switch (c)
        {
        case 'a':
        case 'A': return 10;
        case 'b':
        case 'B': return 11;
        case 'c':
        case 'C': return 12;
        case 'd':
        case 'D': return 13;
        case 'e':
        case 'E': return 14;
        case 'f':
        case 'F': return 15;
        default: assert(false && "not a hex digit");
        }
    };

    auto unescape = [&](const std::string& s) {
        std::string result;

        for (size_t i = 0; i < s.size(); i++)
        {
            if (auto c = s[i]; c != '\\')
            {
                result.push_back(s[i]);
                continue;
            }

            switch (auto escape_character = s[++i]; escape_character)
            {
            case 'n': result.push_back('\n'); break;
            case '\\': result.push_back('\\'); break;
            case 'b': result.push_back('\b'); break;
            case 't': result.push_back('\t'); break;
            case '"': result.push_back('\"'); break;
            case 'x':
            {
                char hex_value    = 0;
                size_t num_digits = 0;

                for (;;)
                {
                    char hex_digit = s[i + 1];
                    if (!hex_digit || !is_hex_digit(hex_digit)) break;
                    num_digits += 1;
                    i += 1;
                }

                for (size_t x = num_digits; x > 0; x--)
                {
                    hex_value |= to_hex_value(s[i - x + 1]) << (4 * (x - 1));
                }

                result.push_back(hex_value);
                break;
            }
            default:
            {
                parser_error(token.span, "Invalid escape character");
            }
            }
        }

        return result;
    };

    return ast::make_node<ast::string>(
        token.span,
        unescape(token.span.to_string()));
}

ast::expression_ptr parser::array(token lparen) noexcept
{
    std::vector<ast::expression_ptr> elems;

    for (;;)
    {
        auto token = _lexer.next_token();
        if (!token) break;

        if (match(token, token_type::rparen))
        {
            _lexer.push_back(token.value());
            break;
        }

        auto expr = expression(token.value());
        if (!expr) return nullptr;

        elems.push_back(std::move(expr));

        if (auto comma = _lexer.next_token(); !match(comma, token_type::comma))
        {
            _lexer.push_back(comma.value());
            break;
        }
    }

    if (auto rparen = _lexer.next_token(); !match(rparen, token_type::rparen))
    {
        parser_error(lparen.span, "Missing ')' after array literal");
        return nullptr;
    }

    return ast::make_node<ast::array>(lparen.span, std::move(elems));
}

}
