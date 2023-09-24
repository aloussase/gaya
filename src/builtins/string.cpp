#include <cstring>

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
    if (!IS_STRING(args[0]))
    {
        interp.interp_error(span, "Expected the first argument to be a string");
        return gaya::eval::object::invalid;
    }

    auto& s1 = AS_STRING(args[0]);
    auto s2
        = IS_STRING(args[1]) ? AS_STRING(args[1]) : to_string(interp, args[1]);

    s2.insert(s2.begin(), s1.begin(), s1.end());

    return create_string(interp, span, std::move(s2));
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

    auto pos   = static_cast<int>(AS_NUMBER(start));
    auto count = static_cast<int>(AS_NUMBER(finish) - pos);

    if (pos < 0                                           //
        || static_cast<size_t>(pos) > AS_STRING(s).size() //
        || count < 0                                      //
        || static_cast<size_t>(pos) + count > AS_STRING(s).size())
    {
        return create_unit(span);
    }

    auto substr = std::string_view {
        AS_STRING(s).data() + pos,
        static_cast<size_t>(count),
    };

    return create_string(interp, span, std::move(substr));
}

gaya::eval::object::object startswith(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    auto& s       = args[0];
    auto& pattern = args[1];
    auto& pos     = args[2];

    if (!IS_STRING(s) || !IS_STRING(pattern))
    {
        interp.interp_error(
            span,
            "Expected the first 2 arguments to be strings");
        return invalid;
    }

    if (!IS_NUMBER(pos))
    {
        interp.interp_error(span, "Expected the 3rd argument to be a number");
        return invalid;
    }

    if (AS_NUMBER(pos) < 0 || AS_NUMBER(pos) > AS_STRING(s).size())
    {
        return create_unit(span);
    }

    if (AS_STRING(s).size() < AS_STRING(pattern).size())
    {
        return create_unit(span);
    }

    auto cmp = std::memcmp(
        AS_STRING(s).c_str() + static_cast<size_t>(AS_NUMBER(pos)),
        AS_STRING(pattern).c_str(),
        AS_STRING(pattern).size());

    return create_number(span, cmp == 0 ? 1 : 0);
}

gaya::eval::object::object endswith(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    auto& s       = args[0];
    auto& pattern = args[1];

    if (!IS_STRING(s) || !IS_STRING(pattern))
    {
        interp.interp_error(span, "Expected both arguments to be strings");
        return invalid;
    }

    if (AS_STRING(s).size() < AS_STRING(pattern).size())
    {
        return create_unit(span);
    }

    auto size = AS_STRING(pattern).size();
    auto cmp  = std::memcmp(
        (AS_STRING(s).end() - size).base(),
        AS_STRING(pattern).c_str(),
        size);

    return create_number(span, cmp == 0 ? 1 : 0);
}

gaya::eval::object::object
trim(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
#define IS_WS(c) ((c) == ' ' || (c) == '\n' || (c) == '\t')

    auto& s = args[0];

    if (!IS_STRING(s))
    {
        interp.interp_error(span, "Expected first argument to be a string");
        return invalid;
    }

    size_t i = 0;
    size_t j = AS_STRING(s).size() - 1;

    while (IS_WS(AS_STRING(s)[i]))
    {
        i += 1;
    }

    while (IS_WS(AS_STRING(s)[j]))
    {
        j -= 1;
    }

    auto trimmed = AS_STRING(s).substr(i, j - i + 1);
    return create_string(interp, span, std::move(trimmed));
#undef IS_WS
}

}
