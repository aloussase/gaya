#include <cassert>
#include <climits>

#include <fmt/core.h>

#include <builtins/array.hpp>
#include <builtins/core.hpp>
#include <builtins/io.hpp>
#include <builtins/sequence.hpp>
#include <builtins/string.hpp>
#include <eval.hpp>
#include <file_reader.hpp>
#include <parser.hpp>
#include <span.hpp>

namespace gaya::eval
{

using namespace std::string_literals;

interpreter::interpreter(const std::string& filename, const char* source)
    : _filename { filename }
    , _source { source }
{
#define BUILTIN(name, arity, func) \
    define(name, create_builtin_function(*this, name, arity, func))

    _scopes.push_back(env {});

    using namespace object::builtin;

    BUILTIN("typeof"s, 1, core::typeof_);
    BUILTIN("assert"s, 1, core::assert_);
    BUILTIN("tostring"s, 1, core::tostring);
    BUILTIN("tosequence"s, 1, core::tosequence);
    BUILTIN("issequence"s, 1, core::issequence);

    BUILTIN("io.println"s, 1, io::println);

    BUILTIN("string.length"s, 1, string::length);
    BUILTIN("string.concat"s, 2, string::concat);

    BUILTIN("array.length"s, 1, array::length);
    BUILTIN("array.concat"s, 2, array::concat);
    BUILTIN("array.push"s, 2, array::push);

    BUILTIN("seq.next"s, 1, sequence::next);
    BUILTIN("seq.make"s, 1, sequence::make);

    if (!loadfile(GAYA_STDLIB_PATH))
    {
        fprintf(stderr, "warning: stdlib could not be loaded\n");
    }

#undef BUILTIN
}

object::maybe_object interpreter::eval() noexcept
{
    auto p   = parser(_source);
    auto ast = p.parse();
    if (!p.diagnostics().empty())
    {
        _diagnostics = p.diagnostics();
        return {};
    }
    return eval(current_env(), std::move(ast));
}

object::maybe_object interpreter::eval(env env, ast::node_ptr ast) noexcept
{
    // NOTE: We don't pop the scope because the repl needs it to keep state.
    current_env() = env;
    return ast->accept(*this);
}

bool interpreter::loadfile(const std::string& filename) noexcept
{
    file_reader reader { filename };
    if (!reader) return false;

    auto* contents = reader.slurp();
    if (strlen(contents) == 0)
    {
        free(contents);
        return true;
    }

    assert(contents);

    auto p   = parser(contents);
    auto ast = p.parse();

    if (ast && p.diagnostics().empty())
    {
        // NOTE: We are leaking contents on purpose here.
        // FIXME: Maybe there is a way to avoid leaking?
        auto old_filename = _filename;
        _filename         = filename;
        (void)eval(current_env(), std::move(ast));
        _filename = old_filename;
        return !had_error();
    }
    else
    {
        for (const auto& diag : p.diagnostics())
        {
            _diagnostics.push_back(diag);
        }
        free(contents);
        return false;
    }
}

std::vector<diagnostic::diagnostic> interpreter::diagnostics() const noexcept
{
    return _diagnostics;
}

const std::string& interpreter::current_filename() const noexcept
{
    return _filename;
}

void interpreter::clear_diagnostics() noexcept
{
    _diagnostics.clear();
}

void interpreter::interp_error(span s, const std::string& msg)
{
    _diagnostics.emplace_back(s, msg, diagnostic::severity::error, _filename);
}

void interpreter::interp_hint(span s, const std::string& hint)
{
    _diagnostics.emplace_back(s, hint, diagnostic::severity::hint, _filename);
}

bool interpreter::had_error() const noexcept
{
    return !_diagnostics.empty();
}

const std::vector<env>& interpreter::scopes() const noexcept
{
    return _scopes;
}

const env& interpreter::get_env() const noexcept
{
    return _scopes.back();
}

env& interpreter::current_env() noexcept
{
    return _scopes.back();
}

void interpreter::define(key k, object::object v) noexcept
{
    current_env().set(std::move(k), v);
}

void interpreter::begin_scope(env new_scope) noexcept
{
    _scopes.push_back(new_scope);
}

void interpreter::end_scope() noexcept
{
    _scopes.pop_back();
}

object::maybe_object interpreter::visit_program(ast::program& program)
{
    for (const auto& stmt : program.stmts)
    {
        stmt->accept(*this);
        if (had_error()) return {};
    }
    return {};
}

object::maybe_object
interpreter::visit_declaration_stmt(ast::declaration_stmt& declaration_stmt)
{
    auto value = declaration_stmt.expr->accept(*this);
    if (value)
    {
        define(key::global(declaration_stmt.ident->value), *value);
    }

    return {};
}

object::maybe_object
interpreter::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
    expression_stmt.expr->accept(*this);
    return {};
}

object::maybe_object
interpreter::visit_assignment_stmt(ast::assignment_stmt& assignment)
{
    auto new_val = assignment.expr->accept(*this);
    if (!new_val) return {};

    auto key = assignment.ident->key;
    key.kind = identifier_kind::local;

    if (!current_env().can_assign_to(key))
    {
        interp_error(
            assignment.ident->_span,
            "Can only assign to local variables");
        interp_hint(
            assignment.ident->_span,
            fmt::format("for example: let {} = ...", assignment.ident->value));
        return {};
    }

    if (!current_env().update_at(std::move(key), new_val.value()))
    {
        interp_error(
            assignment.ident->_span,
            fmt::format(
                "{} was not previously declared",
                assignment.ident->value));
    }

    return {};
}

object::maybe_object interpreter::visit_while_stmt(ast::while_stmt& while_stmt)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (;;)
    {
        auto test = while_stmt.condition->accept(*this);
        if (!test) return {};
        if (!object::is_truthy(test.value())) break;

        for (auto& stmt : while_stmt.body)
        {
            stmt->accept(*this);
            if (had_error()) return {};
        }

        if (while_stmt.continuation)
        {
            while_stmt.continuation->accept(*this);
            if (had_error()) return {};
        }
    }
    end_scope();
    return {};
}

object::maybe_object
interpreter::visit_do_expression(ast::do_expression& do_expr)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (size_t i = 0; i < do_expr.body.size() - 1; i++)
    {
        do_expr.body[i]->accept(*this);
        if (had_error()) return {};
    }
    auto result = do_expr.body.back()->accept(*this);
    end_scope();
    return result;
}

object::maybe_object
interpreter::visit_case_expression(ast::case_expression& cases)
{
    for (const auto& branch : cases.branches)
    {
        auto condition = branch.condition->accept(*this);
        if (!condition) return {};

        if (object::is_truthy(condition.value()))
        {
            auto result = branch.body->accept(*this);
            if (!result) return {};

            return result;
        }
    }

    if (cases.otherwise)
    {
        auto result = cases.otherwise->accept(*this);
        if (!result) return {};

        return result;
    }

    return object::create_unit(cases.span_);
}

object::maybe_object
interpreter::visit_binary_expression(ast::binary_expression& binop)
{
    return binop.execute(*this);
}

object::maybe_object
interpreter::visit_unary_expression(ast::unary_expression& unary_op)
{
    return unary_op.execute(*this);
}

object::maybe_object
interpreter::visit_call_expression(ast::call_expression& cexpr)
{
    auto o = cexpr.target->accept(*this);
    if (!o) return {};

    if (!object::is_callable(o.value()))
    {
        interp_error(cexpr.span_, "Tried to call non-callable");
        interp_hint(
            cexpr.span_,
            "To define a function, do f :: { <args> => <expr> }");
        return {};
    }

    auto arity = object::arity(o.value());

    if (arity != cexpr.args.size())
    {
        interp_error(
            cexpr.span_,
            fmt::format(
                "Wrong number of arguments provided to callable, {} {} "
                "expected",
                arity,
                arity == 1 ? "was" : "were"));
        return {};
    }

    std::vector<object::object> args;
    for (auto& arg : cexpr.args)
    {
        auto result = arg->accept(*this);
        if (!result) return {};

        args.push_back(result.value());
    }

    return object::call(o.value(), *this, cexpr.span_, args);
}

object::maybe_object
interpreter::visit_function_expression(ast::function_expression& fexpr)
{
    std::vector<key> params;
    std::transform(
        fexpr.params.begin(),
        fexpr.params.end(),
        std::back_inserter(params),
        [](auto& param) { return key::param(param.value); });

    return object::create_function(
        *this,
        fexpr._span,
        std::make_unique<env>(std::make_shared<env>(current_env())),
        std::move(params),
        fexpr.body);
}

object::maybe_object
interpreter::visit_let_expression(ast::let_expression& let_expression)
{
    begin_scope(env { std::make_shared<env>(current_env()) });

    for (auto& binding : let_expression.bindings)
    {
        auto value = binding.value->accept(*this);
        if (!value) return {};

        define(key::local(binding.ident->value), *value);
    }

    auto result = let_expression.expr->accept(*this);

    end_scope();

    return result;
}

object::maybe_object interpreter::visit_array(ast::array& ary)
{
    std::vector<object::object> elems;

    for (auto& elem : ary.elems)
    {
        auto o = elem->accept(*this);
        if (!o) return {};

        elems.push_back(o.value());
    }

    return object::create_array(*this, ary.span_, elems);
}

object::maybe_object interpreter::visit_number(ast::number& n)
{
    return object::create_number(n._span, n.value);
}

object::maybe_object interpreter::visit_string(ast::string& s)
{
    return object::create_string(*this, s._span, s.value);
}

object::maybe_object interpreter::visit_identifier(ast::identifier& identifier)
{
    if (auto value = current_env().get(identifier.key); value)
    {
        return value;
    }

    interp_error(
        identifier._span,
        fmt::format("undefined identifier: {}", identifier.value));
    interp_hint(
        identifier._span,
        fmt::format(
            "Maybe you forgot to declare it? For example: {} :: \"someshit\"",
            identifier.value));

    return {};
}

object::maybe_object interpreter::visit_unit(ast::unit& u)
{
    return object::create_unit(u._span);
}

object::maybe_object interpreter::visit_placeholder(ast::placeholder& p)
{
    static std::string underscore = "_";
    if (auto value = current_env().get(underscore); value)
    {
        return value;
    }
    else
    {
        interp_error(p.span_, "Failed to replace placeholder");
        interp_hint(p.span_, "Placeholders can only be used in pipelines");
        return {};
    }
}

}
