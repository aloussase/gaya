#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>

#include <fmt/core.h>

#include <ast.hpp>
#include <eval.hpp>
#include <object.hpp>
#include <sequence.hpp>

namespace gaya::eval::object
{

sequence_ptr object::to_sequence() noexcept
{
    return nullptr;
}

bool object::is_sequence() const noexcept
{
    return false;
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

std::string function::to_string() noexcept
{
    return fmt::format("<function-{}>", _arity);
}

std::string function::typeof_() const noexcept
{
    return "function";
}

size_t function::arity() const noexcept
{
    return _arity;
}

bool function::is_truthy() const noexcept
{
    return true;
}

bool function::is_comparable() const noexcept
{
    return false;
}

bool function::is_callable() const noexcept
{
    return true;
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

bool function::equals(object_ptr) const noexcept
{
    return false;
}

std::string builtin_function::to_string() noexcept
{
    return fmt::format("<builtin-function: {}>", name);
}

bool builtin_function::is_callable() const noexcept
{
    return true;
}

std::string builtin_function::typeof_() const noexcept
{
    return "builtin-function";
}

bool builtin_function::is_truthy() const noexcept
{
    return true;
}

bool builtin_function::is_comparable() const noexcept
{
    return false;
}

bool builtin_function::equals(object_ptr) const noexcept
{
    return false;
}

std::string array::to_string() noexcept
{
    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < elems.size(); i++)
    {
        ss << elems[i]->to_string();
        if (i < elems.size() - 1)
        {
            ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}

bool array::is_callable() const noexcept
{
    return true;
}

std::string array::typeof_() const noexcept
{
    return "array";
}

bool array::is_truthy() const noexcept
{
    return !elems.empty();
}

bool array::is_comparable() const noexcept
{
    return false;
}

size_t array::arity() const noexcept
{
    return 1;
}

object_ptr array::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    auto i = args[0];
    if (i->typeof_() != "number")
    {
        interp.interp_error(span, "Can only index arrays with numbers");
        return nullptr;
    }

    auto n = std::static_pointer_cast<eval::object::number>(i)->value;
    if (n < 0 || n >= elems.size())
    {
        interp.interp_error(
            span,
            fmt::format(
                "Invalid index for array of size {}: {}",
                elems.size(),
                n));
        return nullptr;
    }

    return elems[n];
}

bool array::equals(object_ptr o) const noexcept
{
    if (typeof_() != o->typeof_()) return false;

    auto other = std::static_pointer_cast<array>(o);

    if (elems.size() != other->elems.size()) return false;

    for (size_t i = 0; i < elems.size(); i++)
    {
        if (!elems[i]->equals(other->elems[i]))
        {
            return false;
        }
    }

    return true;
}

std::string number::to_string() noexcept
{
    // https://stackoverflow.com/questions/1521607/check-double-variable-if-it-contains-an-integer-and-not-floating-point
    double intval;
    auto has_decimals = std::modf(value, &intval) != 0.0;
    return has_decimals ? fmt::format("{}", value)
                        : fmt::format("{:.0f}", value);
}

bool number::is_callable() const noexcept
{
    return false;
}

bool number::is_comparable() const noexcept
{
    return true;
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

bool number::equals(object_ptr o) const noexcept
{
    if (typeof_() != o->typeof_()) return false;
    auto other = std::static_pointer_cast<number>(o);
    return value == other->value;
}

std::string string::to_string() noexcept
{
    return '"' + value + '"';
}

std::string string::typeof_() const noexcept
{
    return "string";
}

bool string::is_truthy() const noexcept
{
    return !value.empty();
}

bool string::is_comparable() const noexcept
{
    return true;
}

bool string::is_callable() const noexcept
{
    return true;
}

size_t string::arity() const noexcept
{
    return 1;
}

object_ptr string::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    auto i = args[0];
    if (i->typeof_() != "number")
    {
        interp.interp_error(span, "Can only index strings with numbers");
        return nullptr;
    }

    auto n = std::static_pointer_cast<eval::object::number>(i)->value;
    if (n < 0 || n >= value.size())
    {
        interp.interp_error(
            span,
            fmt::format(
                "Invalid index for string of size {}: {}",
                value.size(),
                n));
        return nullptr;
    }

    return std::make_shared<eval::object::string>(
        span,
        std::string { value[n] });
}

std::optional<int> string::cmp(object_ptr other) const noexcept
{
    if (other->typeof_() != typeof_()) return std::nullopt;
    auto cmp = value <=> std::static_pointer_cast<string>(other)->value;
    return (cmp < 0) ? -1 : ((cmp == 0) ? 0 : 1);
}

bool string::equals(object_ptr o) const noexcept
{
    if (typeof_() != o->typeof_()) return false;
    auto other = std::static_pointer_cast<string>(o);
    return value == other->value;
}

bool string::is_sequence() const noexcept
{
    return true;
}

sequence_ptr string::to_sequence() noexcept
{
    return std::make_shared<string_sequence>(_span, value);
}

std::string unit::to_string() noexcept
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

bool unit::is_comparable() const noexcept
{
    return true;
}

std::optional<int> unit::cmp(object_ptr other) const noexcept
{
    if (other->typeof_() != typeof_()) return std::nullopt;
    return 0;
}

bool unit::equals(object_ptr o) const noexcept
{
    return typeof_() == o->typeof_();
}

}
