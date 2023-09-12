#include <fmt/core.h>

#include <builtins/string.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::string
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

    if (o->typeof_() != "string")
    {
        interp.interp_error(
            span,
            fmt::format("{} expected its argument to be a string", name));
        return nullptr;
    }

    auto s = std::static_pointer_cast<gaya::eval::object::string>(o);
    auto r = s->value.size();

    return std::make_shared<number>(span, r);
}

}
