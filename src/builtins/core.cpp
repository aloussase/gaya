#include <fmt/core.h>

#include <builtins/core.hpp>
#include <eval.hpp>
#include <object.hpp>
#include <sequence.hpp>

namespace gaya::eval::object::builtin::core
{

size_t typeof_::arity() const noexcept
{
    return 1;
}

object_ptr
typeof_::call(interpreter&, span span, std::vector<object_ptr> args) noexcept
{
    auto& o   = args[0];
    auto type = o->typeof_();
    return std::make_shared<string>(span, type);
}

size_t assert_::arity() const noexcept
{
    return 1;
}

object_ptr assert_::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    if (args[0]->is_truthy())
    {
        return std::make_shared<unit>(span);
    }
    else
    {
        interp.interp_error(
            span,
            fmt::format(
                "assertion failed at {}:{}: {} was false ",
                interp.current_filename(),
                span.lineno(),
                args[0]->to_string()));
        return nullptr;
    }
}

size_t tostring::arity() const noexcept
{
    return 1;
}

object_ptr
tostring::call(interpreter&, span span, std::vector<object_ptr> args) noexcept
{
    if (auto s = std::dynamic_pointer_cast<string>(args[0]); s)
    {
        return s;
    }
    return std::make_shared<string>(span, args[0]->to_string());
}

/* issequence */

size_t issequence::arity() const noexcept
{
    return 1;
}

object_ptr
issequence::call(interpreter&, span span, std::vector<object_ptr> args) noexcept
{
    return std::make_shared<number>(span, args[0]->is_sequence());
}

/* tosequence */

size_t tosequence::arity() const noexcept
{
    return 1;
}

object_ptr tosequence::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    auto sequence = args[0]->to_sequence();
    if (!sequence)
    {
        interp.interp_error(
            span,
            fmt::format(
                "{} can't be converted to a sequence",
                args[0]->to_string()));
    }

    return std::static_pointer_cast<object>(sequence);
}

}
