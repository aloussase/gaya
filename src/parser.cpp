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
    define("io.print"s);
    define("io.readline"s);
    define("io.readfile"s);

    define("string.length"s);
    define("string.concat"s);
    define("string.tonumber"s);
    define("string.index"s);
    define("string.substring"s);
    define("string.startswith"s);
    define("string.endswith"s);
    define("string.trim"s);

    define("array.length"s);
    define("array.concat"s);
    define("array.push"s);
    define("array.pop"s);
    define("array.sort"s);

    define("dict.length"s);
    define("dict.set"s);
    define("dict.remove"s);
    define("dict.contains"s);

    define("seq.next"s);
    define("seq.make"s);

    define("math.ceil"s);
    define("math.floor"s);
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

bool parser::match(std::optional<token> t, token_type tt) noexcept
{
    return t && t->type == tt;
}

bool parser::match(token_type tt) noexcept
{
    auto t = _lexer.peek_token();
    if (!t) return false;

    if (t->type == tt)
    {
        (void)_lexer.next_token();
        return true;
    }

    return false;
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
    case token_type::identifier:
    {
        if (auto colon_colon = _lexer.peek_token();
            match(colon_colon, token_type::colon_colon))
        {
            return declaration_stmt(token.value());
        }
        else
        {
            return expression_stmt(token.value());
        }
    }
    case token_type::include:
    {
        return include_stmt(token.value());
    }
    default:
    {
        return expression_stmt(token.value());
    }
    }
}

ast::stmt_ptr parser::local_stmt(token token) noexcept
{
    switch (token.type)
    {
    case token_type::identifier:
    {
        if (auto backarrow = _lexer.peek_token();
            match(backarrow, token_type::back_arrow))
        {
            return assignment_stmt(token);
        }
        else
        {
            return expression_stmt(token);
        }
    }
    case token_type::while_:
    {
        return while_stmt(token);
    }
    case token_type::for_:
    {
        return for_in_stmt(token);
    }
    default:
    {
        return expression_stmt(token);
    }
    }
}

ast::stmt_ptr parser::assignment_stmt(token identifier) noexcept
{
    assert(identifier.type == token_type::identifier);
    if (!match(token_type::back_arrow))
    {
        parser_error(
            identifier.span,
            "Expected '<-' after identifier in assignment statement");
        return nullptr;
    }

    auto expression_token = _lexer.next_token();
    if (!expression_token)
    {
        parser_error(
            identifier.span,
            "Expected expression after '<-' in assignment statement");
        return nullptr;
    }

    auto expr = expression(expression_token.value());
    if (!expr)
    {
        parser_error(
            identifier.span,
            "Expected an expression after '<-' in assignment statement");
        return nullptr;
    }

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
    assert(while_.type == token_type::while_);

    begin_scope();

    auto span            = while_.span;
    auto condition_token = _lexer.next_token();
    if (!condition_token)
    {
        parser_error(span, "Expected a condition after 'while'");
        end_scope();
        return nullptr;
    }

    auto condition = expression(condition_token.value());
    if (!condition)
    {
        parser_error(span, "Expected a condition after 'while'");
        end_scope();
        return nullptr;
    }

    ast::stmt_ptr continuation = nullptr;

    if (match(token_type::colon))
    {
        if (auto identifier_token = _lexer.next_token();
            match(identifier_token, token_type::identifier))
        {
            continuation = assignment_stmt(identifier_token.value());
        }
        else
        {
            parser_error(
                while_.span,
                "Expected a an assignment statement after ':' in while");
            end_scope();
            return nullptr;
        }
    }

    std::vector<ast::stmt_ptr> body;

    for (;;)
    {
        auto stmt_token = _lexer.next_token();
        if (!stmt_token) break;

        if (match(stmt_token, token_type::end))
        {
            _lexer.push_back(stmt_token.value());
            break;
        }

        auto stmt = local_stmt(stmt_token.value());
        if (!stmt)
        {
            parser_error(
                while_.span,
                "Expected a local statement in while body");
            end_scope();
            return nullptr;
        }

        body.push_back(std::move(stmt));
    }

    end_scope();

    if (auto end_token = _lexer.next_token();
        !match(end_token, token_type::end))
    {
        parser_error(
            span,
            fmt::format(
                "Expected 'end' after while body, but got '{}'",
                end_token ? end_token->span.to_string() : "nothing"));
        parser_hint(span, "You can only use local statements in while body");
        return nullptr;
    }

    return ast::make_node<ast::while_stmt>(
        span,
        condition,
        std::move(body),
        continuation);
}

ast::stmt_ptr parser::for_in_stmt(token for_) noexcept
{
    assert(for_.type == token_type::for_);

    begin_scope();

    auto ident = _lexer.next_token();
    if (!match(ident, token_type::identifier))
    {
        parser_error(for_.span, "Expected an identifier after 'for'");
        end_scope();
        return nullptr;
    }

    auto identifier
        = ast::make_node<ast::identifier>(ident->span, ident->span.to_string());
    define(identifier->key);

    if (!match(token_type::in))
    {
        parser_error(for_.span, "Expected 'in' after identifier in for-loop");
        end_scope();
        return nullptr;
    }

    auto e_t = _lexer.next_token();
    if (!e_t)
    {
        parser_error(
            for_.span,
            "Expected an expression after 'in' in for-loop");
        end_scope();
        return nullptr;
    }

    auto e = expression(*e_t);
    if (!e)
    {
        end_scope();
        return nullptr;
    }

    std::vector<ast::stmt_ptr> body;

    for (;;)
    {
        auto s_t = _lexer.next_token();
        if (!s_t) break;

        if (match(s_t, token_type::end))
        {
            _lexer.push_back(s_t.value());
            break;
        }

        auto stmt = local_stmt(s_t.value());
        if (!stmt)
        {
            parser_error(for_.span, "Expected a local statement in for body");
            end_scope();
            return nullptr;
        }

        body.push_back(std::move(stmt));
    }

    end_scope();

    if (!match(token_type::end))
    {
        parser_error(for_.span, "Expected 'end' after for-loop");
        return nullptr;
    }

    return ast::make_node<ast::for_in_stmt>(
        for_.span,
        std::move(identifier),
        std::move(e),
        std::move(body));
}

ast::stmt_ptr parser::declaration_stmt(token identifier)
{
    auto colon_colon = _lexer.next_token();
    if (!colon_colon || colon_colon->type != token_type::colon_colon)
    {
        parser_error(identifier.span, "Expected a '::' after identifier");
        return nullptr;
    }

    auto ident = std::make_unique<ast::identifier>(
        identifier.span,
        identifier.span.to_string());

    /*
     * Here we are defining the identifier before evaluating its expression,
     * which means that we do support top level recursive declarations.
     *
     * NOTE: We still need to add support for this in the evaluator.
     * NOTE: I might not have updated this comment since adding support to the
     *       evaluator.
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

ast::stmt_ptr parser::expression_stmt(token token)
{
    auto expr = expression(token);
    if (!expr) return nullptr;

    if (!match(token_type::dot))
    {
        parser_error(
            token.span,
            "Expected '.' after expression in expression statement");
        parser_hint(token.span, "Try adding a '.' after the expression");
        return nullptr;
    }

    return ast::make_node<ast::expression_stmt>(expr);
}

ast::stmt_ptr parser::include_stmt(token include) noexcept
{
    auto t = _lexer.next_token();
    if (!match(t, token_type::string))
    {
        parser_error(include.span, "Expected a string after include");
        return nullptr;
    }

    std::filesystem::path file_path = t->span.to_string();
    auto span                       = include.span;

    if (file_path.has_extension() && file_path.extension() != ".gaya")
    {
        parser_error(span, "Can only include gaya files");
        return nullptr;
    }
    else
    {
        file_path = file_path.replace_extension("gaya");
    }

    if (!std::filesystem::exists(file_path))
    {
        /* If the file is not found, try relative to the script file. */
        auto script_file = std::filesystem::absolute(_filename);
        file_path        = script_file.replace_filename(file_path);
    }

    if (!std::filesystem::exists(file_path))
    {
        file_path = GAYA_RUNTIME / file_path.filename();
    }

    file_reader reader { file_path };
    if (!reader)
    {
        parser_error(
            span,
            fmt::format(
                "Failed to read the contents of {}",
                file_path.string()));
        return nullptr;
    }

    if (_included_files.contains(file_path))
    {
        /* If we've already processed that file, just ignore it. */
        return ast::make_node<ast::expression_stmt>(
            ast::make_node<ast::unit>(span));
    }

    auto* source = reader.slurp();
    assert(source);

    auto lexer    = _lexer;
    auto filename = _filename;

    auto ast = parse(file_path, source);
    if (!ast)
    {
        parser_error(
            span,
            fmt::format(
                "There were errors while parsing {}",
                file_path.string()));
        return nullptr;
    }

    _lexer    = lexer;
    _filename = filename;

    return ast::make_node<ast::include_stmt>(
        include.span,
        t->span.to_string(),
        ast);
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
    assert(lcurly.type == token_type::lcurly);

    begin_scope();

    std::vector<ast::function_param> params;

    for (;;)
    {
        auto p_t = _lexer.next_token();
        if (!p_t || match(p_t, token_type::arrow))
        {
            if (p_t) _lexer.push_back(*p_t);
            break;
        }

        auto p = match_pattern(*p_t, true, &eval::key::param);
        if (!p)
        {
            parser_error(
                p_t->span,
                "Expected a pattern in function parameter list");
            end_scope();
            return nullptr;
        }

        ast::function_param param = { std::move(*p) };
        params.push_back(std::move(param));

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (!match(token_type::arrow))
    {
        parser_error(lcurly.span, "Expected '=>' after function parameters");
        end_scope();
        return nullptr;
    }

    if (auto r_t = _lexer.next_token(); match(r_t, token_type::rcurly))
    {
        end_scope();
        return ast::make_node<ast::function_expression>(
            lcurly.span,
            std::move(params),
            ast::make_node<ast::unit>(r_t->span));
    }
    else if (r_t)
    {
        auto e = expression(*r_t);
        if (!e)
        {
            parser_error(
                lcurly.span,
                "Expected an expression after '=>' in function literal");
            end_scope();
            return nullptr;
        }

        if (!match(token_type::rcurly))
        {
            parser_error(lcurly.span, "Expected a '}' after function body");
            end_scope();
            return nullptr;
        }

        end_scope();
        return ast::make_node<ast::function_expression>(
            lcurly.span,
            std::move(params),
            std::move(e));
    }
    else
    {
        parser_error(lcurly.span, "Expected a '}' after function body");
        end_scope();
        return nullptr;
    }
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

ast::expression_ptr parser::pipe_expression(token token) noexcept
{
    auto lhs = bitwise_expression(token);
    if (!lhs) return nullptr;

    for (;;)
    {
        auto pipe_token = _lexer.next_token();
        if (!match(pipe_token, token_type::pipe))
        {
            if (pipe_token)
            {
                _lexer.push_back(pipe_token.value());
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
        auto rhs = bitwise_expression(expr_token.value());
        end_scope();

        if (!rhs) return nullptr;

        assert(lhs && rhs);

        lhs = ast::make_node<ast::binary_expression>(
            lhs,
            pipe_token.value(),
            rhs);
    }

    return lhs;
}

ast::expression_ptr parser::bitwise_expression(token token) noexcept
{
    auto lhs = term_expression(token);
    if (!lhs) return nullptr;

    auto done = false;
    while (!done)
    {
        auto t = _lexer.next_token();
        if (!t) break;

        switch (t->type)
        {
        case token_type::land:
        case token_type::lor:
        case token_type::xor_:
        case token_type::lshift:
        case token_type::rshift:
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

            auto rhs = term_expression(t.value());
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
        case token_type::diamond:
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

ast::expression_ptr parser::unary_expression(token op) noexcept
{
    switch (op.type)
    {
    case token_type::perform: return perform_expression(op);
    case token_type::not_: return not_expression(op);
    case token_type::lnot: return lnot_expression(op);
    default: return call_expression(op);
    }
}

ast::expression_ptr parser::lnot_expression(token op) noexcept
{
    assert(op.type == token_type::lnot);
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            op.span,
            fmt::format(
                "Expected an expression after '~'",
                op.span.to_string()));
        return nullptr;
    }

    auto expr = call_expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::lnot_expression>(op, expr);
}

ast::expression_ptr parser::not_expression(token op) noexcept
{
    assert(op.type == token_type::not_);
    auto token = _lexer.next_token();
    if (!token)
    {
        parser_error(
            op.span,
            fmt::format(
                "Expected an expression after 'not'",
                op.span.to_string()));
        return nullptr;
    }

    auto expr = call_expression(token.value());
    if (!expr) return nullptr;

    return ast::make_node<ast::not_expression>(op, expr);
}

ast::expression_ptr parser::perform_expression(token op) noexcept
{
    assert(op.type == token_type::perform);
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

ast::expression_ptr parser::call_expression(token starttoken)
{
    auto expr = primary_expression(starttoken);
    if (!expr) return nullptr;

    for (;;)
    {
        auto lparen = _lexer.next_token();
        if (!lparen) break;

        auto has_argument_list = true;
        std::vector<ast::expression_ptr> args;

        if (!match(lparen, token_type::lparen))
        {
            if (match(lparen, token_type::lcurly))
            {
                auto function = function_expression(lparen.value());
                if (!function)
                {
                    parser_error(
                        starttoken.span,
                        "Expected a trailing function in call expression");
                    return nullptr;
                }
                args.push_back(function);
                has_argument_list = false;
            }
            else
            {
                _lexer.push_back(lparen.value());
                break;
            }
        }

        for (;;)
        {
            if (!has_argument_list) break;

            auto expr_token = _lexer.next_token();
            if (!expr_token) break;

            if (match(expr_token, token_type::rparen))
            {
                _lexer.push_back(expr_token.value());
                break;
            }

            auto arg = expression(expr_token.value());
            if (!arg) return nullptr;

            args.push_back(std::move(arg));

            if (!match(token_type::comma))
            {
                break;
            }
        }

        if (!match(token_type::rparen) && has_argument_list)
        {
            parser_error(starttoken.span, "Missing ')' after function call");
            return nullptr;
        }

        if (auto lcurly = _lexer.next_token();
            match(lcurly, token_type::lcurly) && has_argument_list)
        {
            auto function = function_expression(lcurly.value());
            if (!function) return nullptr;
            args.push_back(function);
        }
        else if (lcurly)
        {
            _lexer.push_back(lcurly.value());
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
    assert(let.type == token_type::let);

    begin_scope();

    auto let_binding = [&](token p_t) -> std::optional<ast::let_binding> {
        auto p = match_pattern(p_t, false);
        if (!p) return {};

        if (!match(token_type::equal))
        {
            parser_error(
                p_t.span,
                "Expected '=' after pattern in let expression");
            return std::nullopt;
        }

        auto e_t = _lexer.next_token();
        if (!e_t)
        {
            parser_error(
                p_t.span,
                "Expected an expression after '=' in let expression");
            return std::nullopt;
        }

        auto e = expression(e_t.value());
        if (!e) return std::nullopt;

        if (p->kind == ast::match_pattern::kind::capture)
        {
            auto e_    = std::get<ast::expression_ptr>(p->value);
            auto ident = std::static_pointer_cast<ast::identifier>(e_);
            define(ident->key);
        }

        return ast::let_binding { p_t.span, std::move(*p), std::move(e) };
    };

    std::vector<ast::let_binding> bindings;
    for (;;)
    {
        auto p_t = _lexer.next_token();
        if (!p_t) break;

        if (match(p_t, token_type::in))
        {
            _lexer.push_back(*p_t);
            break;
        }

        auto binding = let_binding(*p_t);
        if (!binding) return nullptr;

        bindings.push_back(std::move(*binding));

        if (!match(token_type::comma))
        {
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

    if (!match(token_type::in))
    {
        parser_error(
            let.span,
            "Expected 'in' after bindings in let expression");
        end_scope();
        return nullptr;
    }

    auto e_t = _lexer.next_token();
    if (!e_t)
    {
        parser_error(let.span, "Expected an expression after 'in'");
        end_scope();
        return nullptr;
    }

    auto e = expression(*e_t);
    if (!e)
    {
        end_scope();
        return nullptr;
    }

    end_scope();

    return ast::make_node<ast::let_expression>(std::move(bindings), e);
}

ast::expression_ptr parser::case_expression(token cases)
{
    std::vector<ast::case_branch> branches;

    for (;;)
    {
        auto g_t = _lexer.next_token();
        if (!g_t) break;

        if (!match(g_t, token_type::given))
        {
            if (match(g_t, token_type::otherwise)
                || match(g_t, token_type::end))
            {
                _lexer.push_back(g_t.value());
                break;
            }
            else
            {
                return match_expression(g_t.value());
            }
        }

        auto c_t       = _lexer.next_token();
        auto condition = c_t ? expression(*c_t) : nullptr;
        if (!condition)
        {
            parser_error(
                g_t->span,
                "Expected an expression after 'given' in cases");
            return nullptr;
        }

        if (!match(token_type::arrow))
        {
            parser_error(g_t->span, "Expected a '=>' after condition in case");
            return nullptr;
        }

        auto e_t  = _lexer.next_token();
        auto body = e_t ? expression(*e_t) : nullptr;
        if (!body)
        {
            parser_error(
                g_t->span,
                "Expected an expression after condition in case");
            return nullptr;
        }

        branches.push_back(ast::case_branch {
            std::move(condition),
            std::move(body),
        });
    }

    ast::expression_ptr otherwise = nullptr;

    if (match(token_type::otherwise))
    {
        if (!match(token_type::arrow))
        {
            parser_error(cases.span, "Expected '=>' after otherwise");
            return nullptr;
        }

        auto e_t  = _lexer.next_token();
        otherwise = e_t ? expression(*e_t) : nullptr;
        if (!otherwise)
        {
            parser_error(cases.span, "Expected an expression after otherwise");
            return nullptr;
        }
    }

    if (!match(token_type::end))
    {
        parser_error(cases.span, "Expected 'end' after cases");
        return nullptr;
    }

    return ast::make_node<ast::case_expression>(
        cases.span,
        std::move(branches),
        otherwise);
}

std::optional<ast::match_pattern> parser::match_pattern(
    const token& pattern_token,
    bool define_matched_identifier,
    std::function<eval::key(const std::string&)> to_key) noexcept
{
    switch (pattern_token.type)
    {
    case token_type::underscore:
    {
        return ast::match_pattern { ast::match_pattern::kind::wildcard };
    }
    case token_type::identifier:
    {
        if (auto t = _lexer.peek_token(); match(t, token_type::lparen))
        {
            /* Not an identifier, a function call. */
            auto e = expression(pattern_token);
            if (!e) return {};

            return ast::match_pattern { ast::match_pattern::kind::expr, e };
        }
        else
        {
            /* Here we have an identifier. */
            auto identifier = ast::make_node<ast::identifier>(
                pattern_token.span,
                pattern_token.span.to_string());
            identifier->key = to_key(identifier->value);

            if (define_matched_identifier)
            {
                /*
                 * We do this conditionally because in let expressions we need
                 * to define the identifier after evaluating the binding's
                 * body.
                 *
                 * This is because the binding value may be a variable of the
                 * same name as the capture pattern.
                 *
                 * This could also technically happen in match expressions, but
                 * we'll see.
                 */
                define(*identifier);
            }

            return ast::match_pattern {
                ast::match_pattern::kind::capture,
                identifier,
            };
        }
    }
    case token_type::lparen:
    {
        /*
         * We are reusing the array syntax for array matches.
         *
         * This means we cannot match on dictionaries. I'll change this
         * according to how useful it proves to match on dictionaries.
         */
        std::vector<ast::match_pattern> patterns;

        for (;;)
        {
            auto t = _lexer.next_token();
            if (!t)
            {
                parser_error(
                    pattern_token.span,
                    "Expected a pattern after '(' in match branch");
                return {};
            }

            if (match(t, token_type::rparen))
            {
                _lexer.push_back(t.value());
                break;
            }

            auto pattern = match_pattern(t.value());
            if (!pattern) return {};

            patterns.push_back(pattern.value());

            if (!match(token_type::comma))
            {
                break;
            }
        }

        if (!match(token_type::rparen))
        {
            parser_error(
                pattern_token.span,
                "Expected a ')' after array pattern in case branch");
            return {};
        }

        return ast::match_pattern {
            ast::match_pattern::kind::array_pattern,
            patterns,
        };
    }
    default:
    {
        /* Otherwise, the pattern is an arbitrary expression. */
        auto e = expression(pattern_token);
        if (!e) return {};

        return ast::match_pattern { ast::match_pattern::kind::expr, e };
    }
    }
}

ast::expression_ptr parser::match_expression(token target)
{
    auto target_expr = expression(target);
    if (!target_expr)
    {
        parser_error(
            target.span,
            "Expected an expression after 'cases' in match expression");
        return nullptr;
    }

    std::vector<ast::match_branch> branches;
    for (;;)
    {
        /* Begin a new scope for the current branch. */
        begin_scope();

        auto given_token = _lexer.next_token();
        if (!match(given_token, token_type::given))
        {
            if (given_token) _lexer.push_back(given_token.value());
            end_scope();
            break;
        }

        auto pattern_token = _lexer.next_token();
        if (!pattern_token)
        {
            parser_error(
                given_token->span,
                "Expected a pattern after 'given' in match expression");
            end_scope();
            return nullptr;
        }

        auto pattern = match_pattern(pattern_token.value());
        if (!pattern)
        {
            parser_error(
                given_token->span,
                "Expected a pattern in match branch");
            end_scope();
            return nullptr;
        }

        /* An optional when clause. */
        ast::expression_ptr condition = nullptr;

        if (match(token_type::when))
        {
            auto when_expr_token = _lexer.next_token();
            if (!when_expr_token)
            {
                parser_error(
                    given_token->span,
                    "Expected an expression after when in match "
                    "expression");
                end_scope();
                return nullptr;
            }

            auto when_expr = expression(when_expr_token.value());
            if (!when_expr)
            {
                parser_error(
                    given_token->span,
                    "Expected an expression after when in match "
                    "expression");
                end_scope();
                return nullptr;
            }

            condition = when_expr;
        }

        /* A mandatory arrow. */
        if (!match(token_type::arrow))
        {
            parser_error(
                given_token->span,
                "Expected a '=>' in match expression branch");
            end_scope();
            return nullptr;
        }

        /* The branch body. */
        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                given_token->span,
                "Expected an expression after '=>' in match branch");
            end_scope();
            return nullptr;
        }

        auto expr = expression(expr_token.value());
        if (!expr)
        {
            parser_error(
                given_token->span,
                "Expected an expression after '=>' in match branch");
            end_scope();
            return nullptr;
        }

        /* End the scope for the branch. */
        end_scope();

        branches.emplace_back(pattern.value(), expr, condition);
    }

    /* An optional otherwise brach. */
    ast::expression_ptr otherwise = nullptr;
    auto otherwise_token          = _lexer.next_token();

    if (match(otherwise_token, token_type::otherwise))
    {
        if (!match(token_type::arrow))
        {
            parser_error(
                otherwise_token->span,
                "Expected a '=>' after otherwise");
            return nullptr;
        }

        auto expr_token = _lexer.next_token();
        if (!expr_token)
        {
            parser_error(
                otherwise_token->span,
                "Expected an expression after otherwise");
            return nullptr;
        }

        auto expr = expression(expr_token.value());
        if (!expr)
        {
            parser_error(
                otherwise_token->span,
                "Expected an expression after otherwise");
            return nullptr;
        }

        otherwise = expr;
    }
    else
    {
        _lexer.push_back(otherwise_token.value());
    }

    /* A mandatory end token. */
    if (auto end = _lexer.next_token(); !match(end, token_type::end))
    {
        parser_error(
            target.span,
            fmt::format(
                "Expected 'end' after match expression, but got '{}'",
                end ? end->span.to_string() : "nothing"));
        return nullptr;
    }

    return ast::make_node<ast::match_expression>(
        target.span,
        target_expr,
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
        auto stmt_token = _lexer.next_token();
        if (!stmt_token) break;

        if (match(stmt_token, token_type::end))
        {
            _lexer.push_back(stmt_token.value());
            break;
        }

        if (stmt_token->type == token_type::while_)
        {
            auto stmt = while_stmt(stmt_token.value());
            if (!stmt) return nullptr;

            body.push_back(stmt);
        }
        else if (stmt_token->type == token_type::for_)
        {
            auto stmt = for_in_stmt(stmt_token.value());
            if (!stmt) return nullptr;

            body.push_back(stmt);
        }
        else if (stmt_token->type == token_type::identifier)
        {
            if (auto lparen_or_backarrow = _lexer.peek_token();
                match(lparen_or_backarrow, token_type::lparen))
            {
                auto e = call_expression(stmt_token.value());
                if (!e) return nullptr;

                if (match(token_type::dot))
                {
                    body.push_back(ast::make_node<ast::expression_stmt>(e));
                }
                else
                {
                    body.push_back(e);
                    parsed_final_expression = true;
                    break;
                }
            }
            else if (match(lparen_or_backarrow, token_type::back_arrow))
            {
                auto stmt = assignment_stmt(stmt_token.value());
                if (!stmt) return nullptr;

                body.push_back(stmt);
            }
            else
            {
                auto e = expression(stmt_token.value());
                if (!e) return nullptr;

                if (match(token_type::dot))
                {
                    body.push_back(ast::make_node<ast::expression_stmt>(e));
                }
                else
                {
                    body.push_back(e);
                    parsed_final_expression = true;
                    break;
                }
            }
        }
        else
        {
            auto expr = expression(stmt_token.value());
            if (!expr)
            {
                parser_error(
                    stmt_token->span,
                    fmt::format(
                        "Expected an expression in do block body, but got '{}'",
                        stmt_token->span.to_string()));
                end_scope();
                return nullptr;
            }

            if (match(token_type::dot))
            {
                body.push_back(ast::make_node<ast::expression_stmt>(expr));
            }
            else
            {
                body.push_back(expr);
                parsed_final_expression = true;
                break;
            }
        }
    }

    if (!parsed_final_expression)
    {
        body.push_back(ast::make_node<ast::unit>(token.span));
    }

    end_scope();

    if (auto end = _lexer.next_token(); !match(end, token_type::end))
    {
        parser_error(
            token.span,
            fmt::format(
                "Expected 'end' after last expression in do block, but got "
                "'{}'",
                end ? end->span.to_string() : "nothing"));
        parser_hint(
            token.span,
            "Check that you didn't forget a '.' after an expression");
        return nullptr;
    }

    return ast::make_node<ast::do_expression>(token.span, std::move(body));
}

ast::expression_ptr parser::primary_expression(token token)
{
    switch (token.type)
    {
    case token_type::number:
    {
        return number(token);
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
        if (match(token_type::thin_arrow))
        {
            return dictionary(token, nullptr);
        }
        else
        {
            // TODO: push_back
            return array(token);
        }
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

bool parser::is_hex_digit(char c) const noexcept
{
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
        || (c >= '0' && c <= '9');
}

int parser::to_hex_value(char c) const noexcept
{
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
    default: assert(0 && "not a hex digit");
    }
}

int parser::hex_string_to_int(const std::string& s, size_t& i) const noexcept
{
    int hex_value     = 0;
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

    return hex_value;
}

ast::expression_ptr parser::string(token token) noexcept
{
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
            case 'x': result.push_back(hex_string_to_int(s, i)); break;
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

ast::expression_ptr parser::number(token token) noexcept
{
    auto value = token.span.to_string();
    double num = 0;

    if (value.starts_with("0x"))
    {
        size_t i = 1;
        num      = hex_string_to_int(value, i);
    }
    else
    {
        num = std::stod(value);
    }

    return ast::make_node<ast::number>(token.span, num);
}

ast::expression_ptr parser::array(token lparen) noexcept
{
    std::vector<ast::expression_ptr> elems;

    for (;;)
    {
        auto expression_token = _lexer.next_token();
        if (!expression_token) break;

        if (match(expression_token, token_type::rparen))
        {
            _lexer.push_back(expression_token.value());
            break;
        }

        auto expr = expression(expression_token.value());
        if (!expr)
        {
            parser_error(
                lparen.span,
                "Expected an expression in array literal");
            return nullptr;
        }

        /*
         * If we see a '->' after the first expression, that means we are
         * parsing a dictionary literal.
         */
        if (match(token_type::thin_arrow))
        {
            return dictionary(lparen, expr);
        }

        elems.push_back(std::move(expr));

        if (!match(token_type::comma))
        {
            break;
        }
    }

    if (auto rparen = _lexer.next_token(); !match(rparen, token_type::rparen))
    {
        parser_error(
            lparen.span,
            fmt::format(
                "Expected a ')' after last expression in array literal, "
                "but "
                "got {}",
                rparen ? '\'' + rparen->span.to_string() + '\'' : "nothing"));
        return nullptr;
    }

    return ast::make_node<ast::array>(lparen.span, std::move(elems));
}

ast::expression_ptr
parser::dictionary(token lparen, ast::expression_ptr first_key) noexcept
{
    std::vector<ast::expression_ptr> keys;
    std::vector<ast::expression_ptr> values;

    if (first_key)
    {
        keys.push_back(std::move(first_key));
    }

    for (;;)
    {
        /* Parse the next value */

        auto expression_token = _lexer.next_token();
        if (!expression_token) break;
        if (match(expression_token, token_type::rparen))
        {
            _lexer.push_back(expression_token.value());
            break;
        }

        auto expr = expression(expression_token.value());
        if (!expr)
        {
            parser_error(
                lparen.span,
                "Expected an expression in dictionary literal");
            return nullptr;
        }

        values.push_back(std::move(expr));

        /* Parse an optional comma */

        if (match(token_type::comma))
        {
            /* Parse the next key */

            auto t = _lexer.next_token();
            if (!t)
            {
                parser_error(lparen.span, "Unterminated dictionary literal");
                return nullptr;
            }

            auto e = expression(t.value());
            if (!e)
            {
                parser_error(
                    lparen.span,
                    "Expected an expression after comma in dictionary "
                    "literal");
                return nullptr;
            }

            keys.push_back(std::move(e));

            /* Parse the arrow after the key */

            auto arrow = _lexer.next_token();
            if (!arrow)
            {
                parser_error(
                    lparen.span,
                    "Expected a '->' after key in dictionary literal");
                return nullptr;
            }
        }
        else
        {
            break;
        }
    }

    if (keys.size() != values.size())
    {
        parser_error(lparen.span, "Unmatched key(s) in dictionary literal");
        return nullptr;
    }

    if (auto rparen = _lexer.next_token(); !match(rparen, token_type::rparen))
    {
        parser_error(
            lparen.span,
            fmt::format(
                "Expected a ')' after last expression in dictionary "
                "literal, "
                "but got {}",
                rparen ? '\'' + rparen->span.to_string() + '\'' : "nothing"));
        return nullptr;
    }

    return ast::make_node<ast::dictionary>(
        lparen.span,
        std::move(keys),
        std::move(values));
}
}
