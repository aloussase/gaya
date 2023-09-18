#include <fmt/core.h>

#include <builtins/sequence.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::sequence
{

/* seq.next */

gaya::eval::object::object
next(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (args[0].type != object_type_sequence)
    {
        auto t = typeof_(args[0]);
        interp.interp_error(span, fmt::format("Expected {} to be sequence", t));
        return gaya::eval::object::invalid;
    }

    return gaya::eval::object::next(interp, AS_SEQUENCE(args[0]));
}

/* seq.make */

gaya::eval::object::object
make(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto next = args[0];

    if (!is_callable(next))
    {
        interp.interp_error(
            span,
            fmt::format("Expected {} to callable", to_string(interp, next)));
        return gaya::eval::object::invalid;
    }

    return create_user_sequence(span, interp, next);
}

}
