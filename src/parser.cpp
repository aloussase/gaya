#include <cassert>
#include <iostream>
#include <memory>

#include <fmt/core.h>

#include <ast.hpp>
#include <parser.hpp>

namespace gaya
{

parser::parser(const char* source)
    : _lexer { source }
{
}

std::vector<diagnostic::diagnostic> parser::diagnostics() const noexcept
{
    return _diagnostics;
}

void parser::parser_error(span s, const std::string& message)
{
    _diagnostics.emplace_back(s, message, diagnostic::severity::error);
}

void parser::parser_hint(span s, const std::string& message)
{
    _diagnostics.emplace_back(s, message, diagnostic::severity::hint);
}

void parser::merge_diagnostics() noexcept
{
    for (const auto& diag : _lexer.diagnostics())
    {
        _diagnostics.insert(_diagnostics.begin(), diag);
    }
}

bool parser::match(std::optional<token> t, token_type tt) const noexcept
{
    return t && t->type() == tt;
}

std::vector<token> parser::remaining_tokens() noexcept
{
    std::vector<token> tokens;
    for (;;)
    {
        auto token = _lexer.next_token();
        if (!token) break;

        tokens.push_back(token.value());
    }
    return tokens;
}

ast::node_ptr parser::parse() noexcept
{
    auto program   = std::make_unique<ast::program>();
    program->stmts = stmts();
    merge_diagnostics();
    return program;
}

ast::expression_ptr parser::parse_expression() noexcept
{
    auto token = _lexer.next_token();
    if (token)
    {
        return expression(token.value());
    }
    return nullptr;
}

ast::stmt_ptr parser::parse_stmt() noexcept
{
    return toplevel_stmt();
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

    switch (token->type())
    {
    case token_type::identifier: return declaration_stmt(token.value());
    case token_type::discard: return expression_stmt(token.value());
    default:
        parser_error(token->get_span(), "Invalid start of top-level statement");
        parser_hint(
            token->get_span(),
            "Only definitions and discard are valid top-level statements");
        return nullptr;
    }
}

ast::stmt_ptr parser::local_stmt(token token) noexcept
{
    switch (token.type())
    {
    case token_type::discard: return expression_stmt(token);
    case token_type::identifier: return assignment_stmt(token);
    default:
        parser_error(token.get_span(), "Invalid start of local stmt");
        return nullptr;
    }
}

ast::stmt_ptr parser::assignment_stmt(token identifier) noexcept
{
    if (auto back_arrow = _lexer.next_token();
        !match(back_arrow, token_type::back_arrow))
    {
        parser_error(
            identifier.get_span(),
            "Expected '<-' after identifier in assignment");
        return nullptr;
    }

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            identifier.get_span(),
            "Expected expression after '<-' in assignment");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    auto ident = std::make_unique<ast::identifier>(
        identifier.get_span(),
        identifier.get_span().to_string());

    return std::make_unique<ast::assignment_stmt>(
        std::move(ident),
        std::move(expr));
}

ast::stmt_ptr parser::declaration_stmt(token identifier)
{
    auto colon_colon = _lexer.next_token();
    if (!colon_colon || colon_colon->type() != token_type::colon_colon)
    {
        parser_error(identifier.get_span(), "Expected a '::' after identifier");
        parser_hint(identifier.get_span(), "Maybe you meant to use discard?");
        return nullptr;
    }

    auto ident = std::make_unique<ast::identifier>(
        identifier.get_span(),
        identifier.get_span().to_string());

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            colon_colon->get_span(),
            "Expected an expression after '::'");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    return std::make_unique<ast::declaration_stmt>(
        std::move(ident),
        std::move(expr));
}

ast::stmt_ptr parser::expression_stmt(token discard)
{
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            discard.get_span(),
            "Expected an expression after discard");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    return std::make_unique<ast::expression_stmt>(std::move(expr));
}

ast::expression_ptr parser::expression(token token)
{
    switch (token.type())
    {
    case token_type::lcurly: return function_expression(token);
    case token_type::let: return let_expression(token);
    case token_type::do_: return do_expression(token);
    case token_type::cases: return case_expression(token);
    default: return comparison_expression(token);
    }
}

ast::expression_ptr parser::function_expression(token lcurly)
{
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

        params.emplace_back(
            identifier->get_span(),
            identifier->get_span().to_string());

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
        parser_error(lcurly.get_span(), "Expected '=>' after function params");
        return nullptr;
    }

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(lcurly.get_span(), "Expected a '}' after function body");
        return nullptr;
    }

    auto expr = ([&]() -> ast::expression_ptr {
        if (match(token, token_type::rcurly))
        {
            return std::make_unique<ast::unit>(token->get_span());
        }
        else
        {
            auto expr = expression(token.value());
            if (auto rcurly = _lexer.next_token();
                !match(rcurly, token_type::rcurly))
            {
                parser_error(
                    lcurly.get_span(),
                    "Expected a '}' after function body");
                return nullptr;
            }
            return expr;
        }
    })();

    if (!expr) return nullptr;

    return std::make_unique<ast::function_expression>(
        lcurly.get_span(),
        std::move(params),
        std::move(expr));
}

ast::expression_ptr parser::comparison_expression(token token) noexcept
{
    auto lhs = call_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type())
        {
        case token_type::less_than:
        case token_type::less_than_eq:
        case token_type::greater_than:
        case token_type::greater_than_eq:
        case token_type::equal_equal:
        {
            auto op = t.value();

            t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    op.get_span(),
                    fmt::format(
                        "Expected an expression after {}",
                        op.get_span().to_string()));
                return nullptr;
            }

            auto rhs = call_expression(t.value());
            if (!rhs) return nullptr;

            // NOTE: Check that this works as intended.
            lhs = std::make_unique<ast::cmp_expression>(
                std::move(lhs),
                op,
                std::move(rhs));
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

ast::expression_ptr parser::call_expression(token tk)
{
    auto token = tk;
    auto expr  = primary_expression(token);
    if (!expr) return nullptr;

    for (;;)
    {
        auto token = _lexer.next_token();
        if (!token) break;

        if (!match(token, token_type::lparen))
        {
            // Not a function call.
            _lexer.push_back(token.value());
            break;
        }

        std::vector<ast::expression_ptr> args;

        for (;;)
        {
            token = _lexer.next_token();
            if (!token) break;

            if (match(token, token_type::rparen))
            {
                _lexer.push_back(token.value());
                break;
            }

            auto arg = expression(token.value());
            if (!arg) return nullptr;

            args.push_back(std::move(arg));

            if (token = _lexer.next_token(); !match(token, token_type::comma))
            {
                _lexer.push_back(token.value());
                break;
            }
        }

        if (token = _lexer.next_token(); !match(token, token_type::rparen))
        {
            parser_error(tk.get_span(), "Missing ')' after function call");
            return nullptr;
        }

        expr = std::make_unique<ast::call_expression>(
            tk.get_span(),
            std::move(expr),
            std::move(args));
    }

    return expr;
}

ast::expression_ptr parser::let_expression(token let)
{
    auto ident = _lexer.next_token();
    if (!match(ident, token_type::identifier))
    {
        parser_error(let.get_span(), "Expected an identifier after 'let'");
        return nullptr;
    }

    auto equal_sign = _lexer.next_token();
    if (!match(equal_sign, token_type::equal))
    {
        parser_error(
            ident->get_span(),
            "Expected '=' after identifier in let expression");
        return nullptr;
    }

    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            equal_sign->get_span(),
            "Expected an expression after '='");
        return nullptr;
    }

    auto binding = expression(token.value());
    if (!binding) return nullptr;

    auto in = _lexer.next_token();
    if (!match(in, token_type::in))
    {
        parser_error(
            token->get_span(),
            "Expected 'in' after expression in let expression");
        return nullptr;
    }

    token = _lexer.next_token();
    if (!token)
    {
        parser_error(in->get_span(), "Expected an expression after 'in'");
        return nullptr;
    }

    auto expr = expression(token.value());
    if (!expr) return nullptr;

    auto identifier = std::make_unique<ast::identifier>(
        ident->get_span(),
        ident->get_span().to_string());

    return std::make_unique<ast::let_expression>(
        std::move(identifier),
        std::move(binding),
        std::move(expr));
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
            parser_error(
                cases.get_span(),
                "Expected a '=>' after condition in case");
            return nullptr;
        }

        token = _lexer.next_token();
        if (!token)
        {
            parser_error(
                cases.get_span(),
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
            parser_error(cases.get_span(), "Expected '=>' after otherwise");
            return nullptr;
        }

        token = _lexer.next_token();
        if (!token)
        {
            parser_error(
                cases.get_span(),
                "Expected and expression after otherwise");
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
        parser_error(cases.get_span(), "Expected 'end' after case");
        return nullptr;
    }

    return std::make_unique<ast::case_expression>(
        cases.get_span(),
        std::move(branches),
        std::move(otherwise));
}

ast::expression_ptr parser::do_expression(token token)
{
    std::vector<ast::node_ptr> body;
    bool parsed_final_expression = false;

    auto is_local_stmt = ([&](auto token) {
        if (token.type() == token_type::discard) return true;
        if (token.type() != token_type::identifier) return false;

        auto lookahead = _lexer.next_token();
        auto ret       = false;

        if (match(lookahead, token_type::back_arrow))
        {
            ret = true;
        }

        _lexer.push_back(lookahead.value());
        return ret;
    });

    for (;;)
    {
        auto tk = _lexer.next_token();
        if (!tk) break;

        if (match(tk, token_type::done))
        {
            _lexer.push_back(tk.value());
            break;
        }

        if (is_local_stmt(tk.value()))
        {
            auto stmt = local_stmt(tk.value());
            if (!stmt) return nullptr;

            body.push_back(std::move(stmt));
        }
        else
        {
            auto expr = expression(tk.value());
            if (!expr) return nullptr;

            body.push_back(std::move(expr));
            parsed_final_expression = true;
            break;
        }
    }

    if (!parsed_final_expression)
    {
        parser_error(token.get_span(), "do blocks must end with an expression");
        parser_hint(token.get_span(), "Maybe remove a discard?");
        return nullptr;
    }

    if (auto tk = _lexer.next_token(); !match(tk, token_type::done))
    {
        parser_error(
            token.get_span(),
            "Expected 'done' after last expression in do block");
        return nullptr;
    }

    return std::make_unique<ast::do_expression>(
        token.get_span(),
        std::move(body));
}

ast::expression_ptr parser::primary_expression(token token)
{
    switch (token.type())
    {
    case token_type::number:
        return std::make_unique<ast::number>(
            token.get_span(),
            std::stod(token.get_span().to_string()));
    case token_type::string:
        return std::make_unique<ast::string>(
            token.get_span(),
            token.get_span().to_string());
    case token_type::identifier:
        return std::make_unique<ast::identifier>(
            token.get_span(),
            token.get_span().to_string());
    case token_type::unit: return std::make_unique<ast::unit>(token.get_span());
    default:
        parser_error(token.get_span(), "Invalid start of primary expression");
        return nullptr;
    }
}

}
