#include <fmt/core.h>

#include <builtins/dict.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::dict
{

gaya::eval::object::object
length(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    using namespace gaya::eval::object;

    auto& d = args[0];

    if (d.type != object_type_dictionary)
    {
        interp.interp_error(span, "Expected first argument to be a dictionary");
        return invalid;
    }

    return create_number(span, AS_DICT(d).size());
}

gaya::eval::object::object
set(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto& d = args[0];
    auto& k = args[1];
    auto& v = args[2];

    if (d.type != object_type_dictionary)
    {
        interp.interp_error(span, "Expected first argument to be a dictionary");
        return invalid;
    }

    AS_DICT(d).insert({ k, v });

    return d;
}

gaya::eval::object::object
remove(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto& d = args[0];
    auto& k = args[1];

    if (d.type != object_type_dictionary)
    {
        interp.interp_error(span, "Expected first argument to be a dictionary");
        return invalid;
    }

    if (auto it = AS_DICT(d).find(k); it != AS_DICT(d).end())
    {
        auto value = it->second;
        AS_DICT(d).erase(it);
        return value;
    }

    interp.interp_error(
        span,
        fmt::format(
            "The key '{}' was not found in the dictionary",
            to_string(interp, k)));

    return invalid;
}

gaya::eval::object::object contains(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    auto& d = args[0];
    auto& k = args[1];

    if (d.type != object_type_dictionary)
    {
        interp.interp_error(span, "Expected first argument to be a dictionary");
        return invalid;
    }

    if (AS_DICT(d).contains(k))
    {
        return create_number(span, 1);
    }
    else
    {
        return create_unit(span);
    }
}

}
