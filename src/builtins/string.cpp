#include <fmt/core.h>

#include <builtins/string.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::string
{

gaya::eval::object::maybe_object length(
    interpreter& interp,
    span span,
    std::vector<gaya::eval::object::object> args) noexcept
{
    auto o = args[0];

    if (o.type != object_type_string)
    {
        interp.interp_error(span, "Expected argument to be a string");
        return {};
    }

    return create_number(span, AS_STRING(o).size());
}

gaya::eval::object::maybe_object concat(
    interpreter& interp,
    span span,
    std::vector<gaya::eval::object::object> args) noexcept
{
    if (args[0].type != object_type_string
        || args[1].type != object_type_string)
    {
        auto t1 = typeof_(args[0]);
        auto t2 = typeof_(args[1]);
        interp.interp_error(
            span,
            fmt::format("Expected {} and {} to be both string", t1, t2));
        return {};
    }

    return create_string(span, AS_STRING(args[0]) + AS_STRING(args[1]));
}

}
