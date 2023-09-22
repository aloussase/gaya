#include <cmath>

#include <builtins/math.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::math
{

gaya::eval::object::object
floor(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto& n = args[0];

    if (!IS_NUMBER(n))
    {
        interp.interp_error(span, "Expected the first argument to be a number");
        return invalid;
    }

    return create_number(span, std::floor(AS_NUMBER(n)));
}

gaya::eval::object::object
ceil(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto& n = args[0];

    if (!IS_NUMBER(n))
    {
        interp.interp_error(span, "Expected the first argument to be a number");
        return invalid;
    }

    return create_number(span, std::ceil(AS_NUMBER(n)));
}

}
