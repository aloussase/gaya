#include <cassert>
#include <climits>

#include <fmt/core.h>

#include <builtins/array.hpp>
#include <builtins/core.hpp>
#include <builtins/io.hpp>
#include <builtins/math.hpp>
#include <builtins/sequence.hpp>
#include <builtins/string.hpp>
#include <eval.hpp>
#include <file_reader.hpp>
#include <parser.hpp>
#include <span.hpp>

namespace gaya::eval
{

interpreter::interpreter(const std::string& filename, const char* source)
    : _filename { filename }
    , _source { source }
{
    /* Create small objects */
    auto s        = span { 0, nullptr, nullptr };
    _true_object  = std::make_shared<object::number>(s, 1);
    _false_object = std::make_shared<object::number>(s, 0);
    _unit_object  = std::make_shared<object::unit>(s);

    /* Zero out the number cache */
    for (int i = 0; i < STATIC_NUMBER_CACHE_SIZE; i++)
    {
        _static_number_cache[i] = nullptr;
    }

    _scopes.push(env {});

    using namespace object::builtin;

    define("typeof", std::make_shared<core::typeof_>());
    define("assert", std::make_shared<core::assert_>());
    define("tostring", std::make_shared<core::tostring>());
    define("issequence", std::make_shared<core::issequence>());
    define("tosequence", std::make_shared<core::tosequence>());

    define("io.println", std::make_shared<io::println>());

    define("math.add", std::make_shared<math::add>());
    define("math.sub", std::make_shared<math::sub>());
    define("math.mult", std::make_shared<math::mult>());
    define("math.div", std::make_shared<math::div>());

    define("string.length", std::make_shared<string::length>());
    define("string.concat", std::make_shared<string::concat>());

    define("array.length", std::make_shared<array::length>());
    define("array.concat", std::make_shared<array::concat>());
    define("array.push", std::make_shared<array::push>());

    define("seq.next", std::make_shared<sequence::next>());
    define("seq.map", std::make_shared<sequence::map>());
    define("seq.make", std::make_shared<sequence::make>());

    if (!loadfile(GAYA_STDLIB_PATH))
    {
        fprintf(stderr, "warning: stdlib could not be loaded\n");
    }
}

std::shared_ptr<o::number>& interpreter::true_object(span span) noexcept
{
    _true_object->_span = span;
    return _true_object;
}

std::shared_ptr<o::number>& interpreter::false_object(span span) noexcept
{
    _false_object->_span = span;
    return _false_object;
}

std::shared_ptr<o::unit>& interpreter::unit_object(span span) noexcept
{
    _unit_object->_span = span;
    return _unit_object;
}

std::shared_ptr<o::number>
interpreter::make_number(span span, double value) noexcept
{
    if (value >= STATIC_NUMBER_CACHE_SIZE || value < 0)
    {
        if (value < INT_MAX)
        {
            auto i = static_cast<int>(value);
            if (_dynamic_number_cache.contains(i))
            {
                return _dynamic_number_cache[i];
            }
            else
            {
                auto newnumber = std::make_shared<object::number>(span, value);
                _dynamic_number_cache[i] = newnumber;
                return newnumber;
            }
        }
        else
        {
            return std::make_shared<object::number>(span, value);
        }
    }

    if (auto cached = _static_number_cache[static_cast<int>(value)]; cached)
    {
        cached->_span = span;
        cached->value = value;
        return cached;
    }
    else
    {
        auto newnumber = std::make_shared<object::number>(span, value);
        _static_number_cache[static_cast<int>(value)] = newnumber;
        return newnumber;
    }
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
    return eval(current_env(), std::move(ast));
}

object::object_ptr interpreter::eval(env env, ast::node_ptr ast) noexcept
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

    assert(contents != nullptr);

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

const env& interpreter::get_env() const noexcept
{
    return _scopes.top();
}

env& interpreter::current_env() noexcept
{
    return _scopes.top();
}

void interpreter::define(const key& k, object::object_ptr v) noexcept
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
        if (had_error()) return nullptr;
    }
    return nullptr;
}

object::object_ptr
interpreter::visit_declaration_stmt(ast::declaration_stmt& declaration_stmt)
{
    auto ident = declaration_stmt.ident->value;
    auto value = declaration_stmt.expr->accept(*this);

    if (value)
    {
        define(key::global(ident), std::shared_ptr { std::move(value) });
    }

    return nullptr;
}

object::object_ptr
interpreter::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
    expression_stmt.expr->accept(*this);
    return nullptr;
}

object::object_ptr
interpreter::visit_assignment_stmt(ast::assignment_stmt& assignment)
{
    auto new_val = assignment.expr->accept(*this);
    if (!new_val) return nullptr;

    auto ident = assignment.ident->value;

    if (!current_env().can_assign_to(ident))
    {
        interp_error(
            assignment.ident->_span,
            "Can only assign to local variables");
        interp_hint(
            assignment.ident->_span,
            fmt::format("for example: let {} = ...", ident));
        return nullptr;
    }

    if (!current_env().update_at(ident, new_val))
    {
        interp_error(
            assignment.ident->_span,
            fmt::format("{} was not previously declared", ident));
    }

    return nullptr;
}

object::object_ptr interpreter::visit_while_stmt(ast::while_stmt& while_stmt)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (;;)
    {
        auto test = while_stmt.condition->accept(*this);
        if (!test) return nullptr;
        if (!test->is_truthy()) break;

        for (auto& stmt : while_stmt.body)
        {
            stmt->accept(*this);
            if (had_error()) return nullptr;
        }

        if (while_stmt.continuation)
        {
            while_stmt.continuation->accept(*this);
            if (had_error()) return nullptr;
        }
    }
    end_scope();
    return nullptr;
}

object::object_ptr interpreter::visit_do_expression(ast::do_expression& do_expr)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (size_t i = 0; i < do_expr.body.size() - 1; i++)
    {
        do_expr.body[i]->accept(*this);
        if (had_error()) return nullptr;
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
interpreter::visit_unary_expression(ast::unary_expression& unary_op)
{
    return unary_op.execute(*this);
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
            "To define a function, do f :: { <args> => <expr> }");
        return nullptr;
    }

    auto callable = std::dynamic_pointer_cast<object::callable>(o);
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
    std::vector<key> params;
    std::transform(
        fexpr.params.begin(),
        fexpr.params.end(),
        std::back_inserter(params),
        [](auto& param) { return key::param(param.value); });

    return std::make_shared<object::function>(
        fexpr._span,
        std::move(params),
        fexpr.body,
        env { std::make_shared<env>(current_env()) });
}

object::object_ptr
interpreter::visit_let_expression(ast::let_expression& let_expression)
{
    begin_scope(env { std::make_shared<env>(current_env()) });

    for (auto& binding : let_expression.bindings)
    {
        auto ident = binding.ident->value;
        auto value = binding.value->accept(*this);
        if (!value) return nullptr;

        define(key::local(ident), value);
    }

    auto result = let_expression.expr->accept(*this);

    end_scope();

    return result;
}

object::object_ptr interpreter::visit_array(ast::array& ary)
{
    std::vector<object::object_ptr> elems;

    for (auto& elem : ary.elems)
    {
        auto o = elem->accept(*this);
        if (!o) return nullptr;

        elems.push_back(o);
    }

    return std::make_shared<object::array>(ary.span_, elems);
}

object::object_ptr interpreter::visit_number(ast::number& n)
{
    return make_number(n._span, n.value);
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

object::object_ptr interpreter::visit_placeholder(ast::placeholder& p)
{
    if (auto value = current_env().get(std::string("_")); value)
    {
        return value;
    }
    else
    {
        interp_error(p.span_, "Failed to replace placeholder");
        interp_hint(p.span_, "Placeholders can only be used in pipelines");
        return nullptr;
    }
}

}
