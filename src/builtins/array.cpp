#include <fmt/core.h>

#include <builtins/array.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::array
{

gaya::eval::object::object
length(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto a = args[0];

    if (!IS_ARRAY(a))
    {
        interp.interp_error(span, "Expected its argument to be an array");
        return gaya::eval::object::invalid;
    }

    return create_number(span, AS_ARRAY(a).size());
}

gaya::eval::object::object
concat(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (!IS_ARRAY(args[0]) || !IS_ARRAY(args[1]))
    {
        auto t1 = typeof_(args[0]);
        auto t2 = typeof_(args[1]);
        interp.interp_error(
            span,
            fmt::format("Expected {} and {} to be both arrays", t1, t2));
        return gaya::eval::object::invalid;
    }

    auto& a1 = AS_ARRAY(args[0]);
    auto& a2 = AS_ARRAY(args[1]);
    a1.insert(a1.cend(), a2.begin(), a2.end());

    return args[0];
}

gaya::eval::object::object
push(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (!IS_ARRAY(args[0]))
    {
        interp.interp_error(span, "Expected first argument to be an array");
        return gaya::eval::object::invalid;
    }

    AS_ARRAY(args[0]).push_back(std::move(args[1]));

    return args[0];
}

gaya::eval::object::object
pop(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (!IS_ARRAY(args[0]))
    {
        interp.interp_error(span, "Expected first argument to be an array");
        return gaya::eval::object::invalid;
    }

    auto& a = AS_ARRAY(args[0]);

    if (a.empty())
    {
        return create_unit(span);
    }

    auto value = a.back();
    a.pop_back();

    return value;
}

}
