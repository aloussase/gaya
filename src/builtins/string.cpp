#include <fmt/core.h>

#include <builtins/string.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::string
{

gaya::eval::object::object length(
    interpreter& interp,
    span span,
    const std::vector<gaya::eval::object::object>& args) noexcept
{
    auto o = args[0];

    if (o.type != object_type_string)
    {
        interp.interp_error(span, "Expected argument to be a string");
        return gaya::eval::object::invalid;
    }

    return create_number(span, AS_STRING(o).size());
}

gaya::eval::object::object concat(
    interpreter& interp,
    span span,
    const std::vector<gaya::eval::object::object>& args) noexcept
{
    if (args[0].type != object_type_string
        || args[1].type != object_type_string)
    {
        auto t1 = typeof_(args[0]);
        auto t2 = typeof_(args[1]);
        interp.interp_error(
            span,
            fmt::format("Expected {} and {} to be both string", t1, t2));
        return gaya::eval::object::invalid;
    }
    auto& s1 = AS_STRING(args[0]);
    auto& s2 = AS_STRING(args[1]);
    s1.append(s2);

    return args[0];
}

}
