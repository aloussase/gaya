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
    if (!IS_STRING(args[0]) || !IS_STRING(args[1]))
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

gaya::eval::object::object tonumber(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    auto& s = args[0];

    if (!IS_STRING(s))
    {
        interp.interp_error(span, "Expected the first argument to be a string");
        return gaya::eval::object::invalid;
    }

    try
    {
        return create_number(span, std::stod(AS_STRING(s)));
    }
    catch (...)
    {
        return create_unit(span);
    }
}

gaya::eval::object::object
index(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto& haystack = args[0];
    auto& needle   = args[1];

    if (!IS_STRING(haystack) || !IS_STRING(needle))
    {
        interp.interp_error(span, "Expected both arguments to be strings");
        return invalid;
    }

    auto pos = AS_STRING(haystack).find(AS_STRING(needle));
    if (pos == std::string::npos)
    {
        return create_unit(span);
    }

    return create_number(span, pos);
}

gaya::eval::object::object substring(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    auto& s      = args[0];
    auto& start  = args[1];
    auto& finish = args[2];

    if (!IS_STRING(s))
    {
        interp.interp_error(span, "Expected the first argument to be a string");
        return invalid;
    }

    if (!IS_NUMBER(start) || !IS_NUMBER(finish))
    {
        interp.interp_error(span, "Expected start and finish to be numbers");
        return invalid;
    }

    auto pos   = AS_NUMBER(start);
    auto count = AS_NUMBER(finish) - pos;

    if (pos < 0 || pos > AS_STRING(s).size() || count < 0
        || pos + count > AS_STRING(s).size())
    {
        return create_unit(span);
    }

    return create_string(interp, span, AS_STRING(s).substr(pos, count));
}

}
