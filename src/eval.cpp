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

#define RETURN_IF_INVALID(o) \
    if (!object::is_valid(o)) return o;

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

std::optional<object::object> interpreter::eval() noexcept
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

std::optional<object::object>
interpreter::eval(env env, ast::node_ptr ast) noexcept
{
    current_env() = env;
    auto result   = ast->accept(*this);

    if (object::is_valid(result))
    {
        return result;
    }
    else
    {
        return {};
    }
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

object::object interpreter::visit_program(ast::program& program)
{
    for (const auto& stmt : program.stmts)
    {
        stmt->accept(*this);
        if (had_error()) return object::invalid;
    }
    return object::invalid;
}

object::object
interpreter::visit_declaration_stmt(ast::declaration_stmt& declaration_stmt)
{
    auto value = declaration_stmt.expr->accept(*this);
    RETURN_IF_INVALID(value);

    define(key::global(declaration_stmt.ident->value), value);

    return object::invalid;
}

object::object
interpreter::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
    expression_stmt.expr->accept(*this);
    /*
     * NOTE: For statements I may want to return object::none or something like
     * that
     */
    return object::invalid;
}

object::object
interpreter::visit_assignment_stmt(ast::assignment_stmt& assignment)
{
    auto new_val = assignment.expr->accept(*this);
    RETURN_IF_INVALID(new_val);

    key key  = assignment.ident->key;
    key.kind = identifier_kind::local;

    if (!current_env().can_assign_to(key))
    {
        interp_error(
            assignment.ident->_span,
            "Can only assign to local variables");

        interp_hint(
            assignment.ident->_span,
            fmt::format("for example: let {} = ...", assignment.ident->value));

        return object::invalid;
    }

    if (!current_env().update_at(std::move(key), new_val))
    {
        interp_error(
            assignment.ident->_span,
            fmt::format(
                "{} was not previously declared",
                assignment.ident->value));
    }

    return object::invalid;
}

object::object interpreter::visit_while_stmt(ast::while_stmt& while_stmt)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (;;)
    {
        auto test = while_stmt.condition->accept(*this);
        if (!object::is_valid(test)) return object::invalid;
        if (!object::is_truthy(test)) break;

        for (auto& stmt : while_stmt.body)
        {
            stmt->accept(*this);
            if (had_error()) return object::invalid;
        }

        if (while_stmt.continuation)
        {
            while_stmt.continuation->accept(*this);
            if (had_error()) return object::invalid;
        }
    }
    end_scope();
    return object::invalid;
}

object::object interpreter::visit_do_expression(ast::do_expression& do_expr)
{
    begin_scope(env { std::make_shared<env>(current_env()) });
    for (size_t i = 0; i < do_expr.body.size() - 1; i++)
    {
        do_expr.body[i]->accept(*this);
        if (had_error()) return object::invalid;
    }
    auto result = do_expr.body.back()->accept(*this);
    end_scope();
    return result;
}

object::object interpreter::visit_case_expression(ast::case_expression& cases)
{
    for (const auto& branch : cases.branches)
    {
        auto condition = branch.condition->accept(*this);
        if (!object::is_valid(condition)) return object::invalid;

        if (object::is_truthy(condition))
        {
            auto result = branch.body->accept(*this);
            if (!object::is_valid(result)) return object::invalid;

            return result;
        }
    }

    if (cases.otherwise)
    {
        auto result = cases.otherwise->accept(*this);
        if (!object::is_valid(result)) return object::invalid;

        return result;
    }

    return object::create_unit(cases.span_);
}

static inline object::object interpret_arithmetic_expression(
    interpreter& interp,
    ast::binary_expression& expr) noexcept
{
    auto l = expr.lhs->accept(interp);
    RETURN_IF_INVALID(l);

    auto r = expr.rhs->accept(interp);
    RETURN_IF_INVALID(r)

    if (!IS_NUMBER(l) || !IS_NUMBER(r))
    {
        interp.interp_error(
            expr.op.span,
            fmt::format(
                "{} expected {} and {} to be both numbers",
                expr.op.span.to_string(),
                object::typeof_(l),
                object::typeof_(r)));
        return object::invalid;
    }

    auto fst = AS_NUMBER(l);
    auto snd = AS_NUMBER(r);

    double result = 0;

    switch (expr.op.type)
    {
    case token_type::plus: result = fst + snd; break;
    case token_type::dash: result = fst - snd; break;
    case token_type::star: result = fst * snd; break;
    case token_type::slash:
    {
        if (snd == 0)
        {
            return object::create_unit(expr.op.span);
        }
        result = fst / snd;
        break;
    }
    default: assert(false && "should not happen");
    }

    return object::create_number(expr.op.span, result);
}

static inline object::object interpret_comparison_expression(
    interpreter& interp,
    ast::binary_expression& expr) noexcept
{
    auto l = expr.lhs->accept(interp);
    RETURN_IF_INVALID(l);

    auto r = expr.rhs->accept(interp);
    RETURN_IF_INVALID(r);

    int result;

    switch (expr.op.type)
    {
    case token_type::equal_equal:
    {
        result = object::equals(l, r) ? 1 : 0;
        break;
    }
    case token_type::not_equals:
    {
        result = !object::equals(l, r) ? 1 : 0;
        break;
    }
    default:
    {
        if (!object::is_comparable(l) || !object::is_comparable(r))
        {
            interp.interp_error(
                expr.op.span,
                fmt::format(
                    "{} and {} are not both comparable",
                    object::typeof_(l),
                    object::typeof_(r)));
            return object::invalid;
        }

        int cmp;
        if (!object::cmp(l, r, &cmp))
        {
            return object::invalid;
        }

        switch (expr.op.type)
        {
        case token_type::less_than: result = cmp < 0 ? 1 : 0; break;
        case token_type::less_than_eq: result = cmp <= 0 ? 1 : 0; break;
        case token_type::greater_than: result = cmp > 0 ? 1 : 0; break;
        case token_type::greater_than_eq: result = cmp >= 0 ? 1 : 0; break;
        default: assert(false && "unreachable");
        }
    }
    }

    return object::create_number(expr.op.span, result);
}

static inline object::object interpret_logical_expression(
    interpreter& interp,
    ast::binary_expression& expr) noexcept
{

    auto l = expr.lhs->accept(interp);
    RETURN_IF_INVALID(l);

    if (expr.op.type == token_type::and_)
    {
        if (object::is_truthy(l))
        {
            return expr.rhs->accept(interp);
        }
        else
        {
            return l;
        }
    }

    if (expr.op.type == token_type::or_)
    {
        if (object::is_truthy(l))
        {
            return l;
        }
        else
        {
            return expr.rhs->accept(interp);
        }
    }

    assert(false && "unreachable");
}

object::object
interpreter::visit_binary_expression(ast::binary_expression& binop)
{
    switch (binop.op.type)
    {
    case token_type::less_than:
    case token_type::less_than_eq:
    case token_type::greater_than:
    case token_type::greater_than_eq:
    case token_type::equal_equal:
    case token_type::not_equals:
    {
        return interpret_comparison_expression(*this, binop);
    }
    case token_type::plus:
    case token_type::star:
    case token_type::dash:
    case token_type::slash:
    {
        return interpret_arithmetic_expression(*this, binop);
    }
    case token_type::pipe:
    {
        auto replacement = binop.lhs->accept(*this);
        RETURN_IF_INVALID(replacement);

        begin_scope(env { std::make_shared<env>(current_env()) });

        static key underscore = key::local("_");
        define(underscore, replacement);

        auto result = binop.rhs->accept(*this);
        RETURN_IF_INVALID(result);

        end_scope();

        return result;
    }
    case token_type::and_:
    case token_type::or_:
    {
        return interpret_logical_expression(*this, binop);
    }
    default:
    {

        assert(0 && "unhandled case in visit_binary_expression");
    }
    }

    assert(0 && "unhandled case in visit_binary_expression");
}

object::object interpreter::visit_not_expression(ast::not_expression& expr)
{
    auto value = expr.operand->accept(*this);
    RETURN_IF_INVALID(value);

    return gaya::eval::object::create_number(
        expr.op.span,
        !gaya::eval::object::is_truthy(value));
}

object::object
interpreter::visit_perform_expression(ast::perform_expression& expr)
{
    expr.stmt->accept(*this);
    if (had_error()) return object::invalid;

    return gaya::eval::object::create_unit(expr.op.span);
}

object::object interpreter::visit_call_expression(ast::call_expression& cexpr)
{
    auto o = cexpr.target->accept(*this);
    RETURN_IF_INVALID(o);

    if (!object::is_callable(o))
    {
        interp_error(cexpr.span_, "Tried to call non-callable");
        interp_hint(
            cexpr.span_,
            "To define a function, do f :: { <args> => <expr> }");
        return object::invalid;
    }

    auto arity = object::arity(o);

    if (arity != cexpr.args.size())
    {
        interp_error(
            cexpr.span_,
            fmt::format(
                "Wrong number of arguments provided to callable, {} {} "
                "expected",
                arity,
                arity == 1 ? "was" : "were"));
        return object::invalid;
    }

    std::vector<object::object> args;
    for (auto& arg : cexpr.args)
    {
        auto result = arg->accept(*this);
        RETURN_IF_INVALID(result);

        args.push_back(result);
    }

    return object::call(o, *this, cexpr.span_, args);
}

object::object
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

object::object
interpreter::visit_let_expression(ast::let_expression& let_expression)
{
    begin_scope(env { std::make_shared<env>(current_env()) });

    for (auto& binding : let_expression.bindings)
    {
        auto value = binding.value->accept(*this);
        RETURN_IF_INVALID(value);

        define(key::local(binding.ident->value), value);
    }

    auto result = let_expression.expr->accept(*this);

    end_scope();

    return result;
}

object::object interpreter::visit_array(ast::array& ary)
{
    std::vector<object::object> elems;

    for (auto& elem : ary.elems)
    {
        auto o = elem->accept(*this);
        RETURN_IF_INVALID(o);

        elems.push_back(o);
    }

    return object::create_array(*this, ary.span_, elems);
}

object::object interpreter::visit_number(ast::number& n)
{
    return object::create_number(n._span, n.value);
}

object::object interpreter::visit_string(ast::string& s)
{
    return object::create_string(*this, s._span, s.value);
}

object::object interpreter::visit_identifier(ast::identifier& identifier)
{
    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (auto value = _scopes[i].get(identifier.key);
            object::is_valid(value))
        {
            return value;
        }
    }

    interp_error(
        identifier._span,
        fmt::format("undefined identifier: {}", identifier.value));

    interp_hint(
        identifier._span,
        fmt::format(
            "Maybe you forgot to declare it? For example: {} :: \"someshit\"",
            identifier.value));

    return object::invalid;
}

object::object interpreter::visit_unit(ast::unit& u)
{
    return object::create_unit(u._span);
}

object::object interpreter::visit_placeholder(ast::placeholder& p)
{
    static key underscore = key::local("_");
    if (auto value = current_env().get(underscore); object::is_valid(value))
    {
        return value;
    }
    else
    {
        interp_error(p.span_, "Failed to replace placeholder");
        interp_hint(p.span_, "Placeholders can only be used in pipelines");
        return object::invalid;
    }
}

}
