#include <cassert>

#include <fmt/core.h>

#include <builtins/core.hpp>
#include <builtins/io.hpp>
#include <builtins/math.hpp>
#include <eval.hpp>
#include <parser.hpp>
#include <span.hpp>

namespace gaya::eval
{

interpreter::interpreter(const char* source)
    : _source { source }
{
}

object::object_ptr interpreter::eval() noexcept
{
    auto p   = parser(_source);
    auto ast = p.parse();
    if (!p.diagnostics().empty())
    {
        _diagnostics = p.diagnostics();
        return nullptr;
    }
    return eval(env {}, std::move(ast));
}

object::object_ptr interpreter::eval(env env, ast::node_ptr ast) noexcept
{
    _scopes.push(env);
    define("io.println", std::make_shared<object::builtin::io::println>());
    define("math.add", std::make_shared<object::builtin::math::add>());
    define("math.sub", std::make_shared<object::builtin::math::sub>());
    define("math.mult", std::make_shared<object::builtin::math::mult>());
    define("math.div", std::make_shared<object::builtin::math::div>());
    define("typeof", std::make_shared<object::builtin::core::typeof_>());
    return ast->accept(*this);
}

std::vector<diagnostic::diagnostic> interpreter::diagnostics() const noexcept
{
    return _diagnostics;
}

void interpreter::clear_diagnostics() noexcept
{
    _diagnostics.clear();
}

void interpreter::interp_error(span s, const std::string& msg)
{
    _diagnostics.emplace_back(s, msg, diagnostic::severity::error);
}

void interpreter::interp_hint(span s, const std::string& hint)
{
    _diagnostics.emplace_back(s, hint, diagnostic::severity::hint);
}

const env& interpreter::get_env() const noexcept
{
    return _scopes.top();
}

env& interpreter::current_env() noexcept
{
    return _scopes.top();
}

void interpreter::define(const std::string& k, object::object_ptr v) noexcept
{
    current_env().set(k, v);
}

void interpreter::begin_scope(env new_scope) noexcept
{
    _scopes.push(new_scope);
}

void interpreter::end_scope() noexcept
{
    _scopes.pop();
}

object::object_ptr interpreter::visit_program(ast::program& program)
{
    for (const auto& stmt : program.stmts)
    {
        stmt->accept(*this);
    }
    return nullptr;
}

object::object_ptr
interpreter::visit_declaration_stmt(ast::declaration_stmt& declaration_stmt)
{
    auto ident = declaration_stmt._identifier->value;
    auto value = declaration_stmt.expression->accept(*this);

    if (value)
    {
        define(ident, std::shared_ptr { std::move(value) });
    }

    return nullptr;
}

object::object_ptr
interpreter::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
    expression_stmt.expr->accept(*this);
    return nullptr;
}

object::object_ptr interpreter::visit_do_expression(ast::do_expression& do_expr)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (size_t i = 0; i < do_expr.body.size() - 1; i++)
    {
        do_expr.body[i]->accept(*this);
    }
    auto result = do_expr.body.back()->accept(*this);
    end_scope();
    return result;
}

object::object_ptr
interpreter::visit_case_expression(ast::case_expression& cases)
{
    for (const auto& branch : cases.branches)
    {
        auto condition = branch.condition->accept(*this);
        if (!condition) return nullptr;

        if (condition->is_truthy())
        {
            auto result = branch.body->accept(*this);
            if (!result) return nullptr;

            return result;
        }
    }

    if (cases.otherwise)
    {
        auto result = cases.otherwise->accept(*this);
        if (!result) return nullptr;

        return result;
    }

    return std::make_shared<object::unit>(cases.span_);
}

object::object_ptr
interpreter::visit_binary_expression(ast::binary_expression& binop)
{
    return binop.execute(*this);
}

object::object_ptr
interpreter::visit_call_expression(ast::call_expression& cexpr)
{
    auto o = cexpr.target->accept(*this);
    if (!o) return nullptr;

    if (!o->is_callable())
    {
        interp_error(cexpr.span_, "Tried to call non-callable");
        interp_hint(
            cexpr.span_,
            "To define a function, do f :: {{ <args> => <expr> }}");
        return nullptr;
    }

    auto callable = std::static_pointer_cast<object::callable>(o);
    if (callable->arity() != cexpr.args.size())
    {
        interp_error(
            cexpr.span_,
            fmt::format(
                "Wrong number of arguments provided to callable, {} {} "
                "expected",
                callable->arity(),
                callable->arity() == 1 ? "was" : "were"));
        return nullptr;
    }

    std::vector<object::object_ptr> args;
    for (auto& arg : cexpr.args)
    {
        auto result = arg->accept(*this);
        if (!result) return nullptr;

        args.push_back(std::move(result));
    }

    return callable->call(*this, cexpr.span_, args);
}

object::object_ptr
interpreter::visit_function_expression(ast::function_expression& fexpr)
{
    return std::make_shared<object::function>(
        fexpr._span,
        fexpr.params,
        fexpr.body,
        env { std::make_shared<env>(current_env()) });
}

object::object_ptr
interpreter::visit_let_expression(ast::let_expression& let_expression)
{
    auto ident   = let_expression.ident->value;
    auto binding = let_expression.binding->accept(*this);
    if (!binding) return nullptr;

    begin_scope(env { std::make_shared<env>(current_env()) });
    define(ident, binding);
    auto result = let_expression.expr->accept(*this);
    end_scope();

    return result;
}

object::object_ptr interpreter::visit_number(ast::number& n)
{
    return std::make_shared<object::number>(n._span, n.value);
}

object::object_ptr interpreter::visit_string(ast::string& s)
{
    return std::make_shared<object::string>(s._span, s.value);
}

object::object_ptr interpreter::visit_identifier(ast::identifier& identifier)
{
    auto ident = identifier.value;
    if (auto value = current_env().get(ident); value)
    {
        return value;
    }

    interp_error(
        identifier._span,
        fmt::format("undefined identifier: {}", ident));
    interp_hint(
        identifier._span,
        fmt::format(
            "Maybe you forgot to declare it? For example: {} :: \"someshit\"",
            ident));

    return nullptr;
}

object::object_ptr interpreter::visit_unit(ast::unit& u)
{
    return std::make_shared<object::unit>(u._span);
}

}
