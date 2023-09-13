#include <fmt/core.h>

#include <builtins/array.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::array
{

size_t length::arity() const noexcept
{
    return 1;
}

object_ptr length::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    auto o = args[0];

    if (o->typeof_() != "array")
    {
        interp.interp_error(
            span,
            fmt::format("{} expected its argument to be an array", name));
        return nullptr;
    }

    auto a = std::static_pointer_cast<gaya::eval::object::array>(o);
    auto n = a->elems.size();

    return std::make_shared<number>(span, n);
}

}
