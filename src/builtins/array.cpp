#include <fmt/core.h>

#include <builtins/array.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::array
{

gaya::eval::object::object
length(interpreter& interp, span span, std::vector<object> args) noexcept
{
    auto o = args[0];

    if (o.type != object_type_array)
    {
        interp.interp_error(span, "Expected its argument to be an array");
        return gaya::eval::object::invalid;
    }

    return create_number(span, AS_ARRAY(o).size());
}

gaya::eval::object::object
concat(interpreter& interp, span span, std::vector<object> args) noexcept
{
    if (args[0].type != object_type_array || args[1].type != object_type_array)
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
push(interpreter& interp, span span, std::vector<object> args) noexcept
{
    if (args[0].type != object_type_array)
    {
        interp.interp_error(span, "Expected first argument to be an array");
        return gaya::eval::object::invalid;
    }

    auto& ary  = args[0];
    auto value = args[1];

    AS_ARRAY(ary).push_back(value);

    return ary;
}

gaya::eval::object::object
pop(interpreter& interp, span span, std::vector<object> args) noexcept
{
    if (args[0].type != object_type_array)
    {
        interp.interp_error(span, "Expected first argument to be an array");
        return gaya::eval::object::invalid;
    }

    auto& ary = AS_ARRAY(args[0]);

    if (ary.empty())
    {
        return create_unit(span);
    }

    auto value = ary.back();
    ary.pop_back();

    return value;
}

}
