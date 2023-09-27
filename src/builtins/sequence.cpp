#include <fmt/core.h>

#include <builtins/sequence.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::sequence
{

/* seq.next */

gaya::eval::object::object
next(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (IS_UNIT(args[0]))
    {
        return create_unit(span);
    }

    if (!IS_SEQUENCE(args[0]))
    {
        auto t = typeof_(args[0]);
        interp.interp_error(
            span,
            fmt::format("Expected {} to be a Sequence", t));
        return gaya::eval::object::invalid;
    }

    return gaya::eval::object::next(interp, AS_SEQUENCE(args[0]));
}

/* seq.make */

gaya::eval::object::object
make(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto cb = args[0];

    if (!is_callable(cb))
    {
        interp.interp_error(
            span,
            fmt::format("Expected {} to be callable", typeof_(cb)));
        return gaya::eval::object::invalid;
    }

    return create_user_sequence(span, interp, cb);
}

/* seq.copy */

gaya::eval::object::object
copy(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (!is_sequence(args[0]))
    {
        interp.interp_error(span, "Expected the first argument to be sequence");
        return invalid;
    }

    if (!IS_SEQUENCE(args[0]))
    {
        auto o = args[0];
        return to_sequence(interp, o);
    }

    return copy_sequence(interp, span, AS_SEQUENCE(args[0]));
}

}
