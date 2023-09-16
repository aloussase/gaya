#include <fmt/core.h>

#include <builtins/core.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object::builtin::core
{

gaya::eval::object::maybe_object
typeof_(interpreter&, span span, std::vector<object> args) noexcept
{
    auto& o   = args[0];
    auto type = gaya::eval::object::typeof_(o);
    return create_string(span, type);
}

gaya::eval::object::maybe_object
assert_(interpreter& interp, span span, std::vector<object> args) noexcept
{
    using namespace gaya::eval::object;

    if (is_truthy(args[0]))
    {
        return create_unit(span);
    }
    else
    {
        interp.interp_error(
            span,
            fmt::format(
                "assertion failed at {}:{}: {} was false ",
                interp.current_filename(),
                span.lineno(),
                to_string(args[0])));
        return {};
    }
}

gaya::eval::object::maybe_object
tostring(interpreter&, span span, std::vector<object> args) noexcept
{
    if (args[0].type == gaya::eval::object::object_type_string)
    {
        return args[0];
    }
    return create_string(span, gaya::eval::object::to_string(args[0]));
}

/* issequence */

gaya::eval::object::maybe_object
issequence(interpreter&, span span, std::vector<object> args) noexcept
{
    return create_number(span, gaya::eval::object::is_sequence(args[0]));
}

/* tosequence */

gaya::eval::object::maybe_object
tosequence(interpreter& interp, span span, std::vector<object> args) noexcept
{
    using namespace gaya::eval::object;

    if (!is_sequence(args[0]))
    {
        interp.interp_error(
            span,
            fmt::format(
                "{} can't be converted to a sequence",
                to_string(args[0])));
    }

    auto sequence = to_sequence(args[0]);
    return sequence;
}

}
