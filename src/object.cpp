#include <cmath>
#include <iostream>

#include <fmt/core.h>

#include <ast.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

bool callable::is_callable() const noexcept
{
    return true;
}

bool callable::is_comparable() const noexcept
{
    return false;
}

std::string callable::typeof_() const noexcept
{
    return "callable";
}

bool callable::is_truthy() const noexcept
{
    return true;
}

bool comparable::is_comparable() const noexcept
{
    return true;
}

function::function(
    span s,
    std::vector<key> p,
    std::shared_ptr<ast::expression> b,
    eval::env e)
    : _span { s }
    , params { std::move(p) }
    , _arity { params.size() }
    , body { b }
    , closed_over_env { std::move(e) }
{
}

std::string function::to_string() const noexcept
{
    return fmt::format("<function-{}>", _arity);
}

size_t function::arity() const noexcept
{
    return _arity;
}

object_ptr
function::call(interpreter& interp, span, std::vector<object_ptr> args) noexcept
{
    interp.begin_scope(env { std::make_shared<env>(closed_over_env) });
    for (size_t i = 0; i < args.size(); i++)
    {
        auto ident = params[i];
        auto arg   = args[i];
        interp.define(ident, arg);
    }
    auto ret = body->accept(interp);
    interp.end_scope();
    return ret;
}

std::string builtin_function::to_string() const noexcept
{
    return fmt::format("<builtin-function: {}>", name);
}

std::string number::to_string() const noexcept
{
    // https://stackoverflow.com/questions/1521607/check-double-variable-if-it-contains-an-integer-and-not-floating-point
    double intval;
    auto has_decimals = std::modf(value, &intval) != 0.0;
    return has_decimals ? fmt::format("{:.2f}", value)
                        : fmt::format("{:.0f}", value);
}

bool number::is_callable() const noexcept
{
    return false;
}

std::string number::typeof_() const noexcept
{
    return "number";
}

bool number::is_truthy() const noexcept
{
    return value != 0.0;
}

std::optional<int> number::cmp(object_ptr other) const noexcept
{
    if (other->typeof_() != typeof_()) return std::nullopt;
    auto other_value = std::static_pointer_cast<number>(other)->value;
    return (value < other_value) ? -1 : ((value == other_value) ? 0 : 1);
}

std::string string::to_string() const noexcept
{
    return value;
}

bool string::is_callable() const noexcept
{
    return false;
}

std::string string::typeof_() const noexcept
{
    return "string";
}

bool string::is_truthy() const noexcept
{
    return !value.empty();
}

std::optional<int> string::cmp(object_ptr other) const noexcept
{
    if (other->typeof_() != typeof_()) return std::nullopt;
    auto cmp = value <=> std::static_pointer_cast<string>(other)->value;
    return (cmp < 0) ? -1 : ((cmp == 0) ? 0 : 1);
}

std::string unit::to_string() const noexcept
{
    return "unit";
}

bool unit::is_callable() const noexcept
{
    return false;
}

std::string unit::typeof_() const noexcept
{
    return "unit";
}

bool unit::is_truthy() const noexcept
{
    return false;
}

std::optional<int> unit::cmp(object_ptr other) const noexcept
{
    if (other->typeof_() != typeof_()) return std::nullopt;
    return 0;
}

}
