#include <eval.hpp>

#include <builtins/math.hpp>

namespace gaya::eval::object::builtin::math
{

size_t add::arity() const noexcept
{
    return 2;
}

object_ptr
add::call(interpreter& interp, span span, std::vector<object_ptr> args) noexcept
{
    auto fst = std::dynamic_pointer_cast<number>(args[0]);
    auto snd = std::dynamic_pointer_cast<number>(args[1]);

    if (!fst || !snd)
    {
        interp.interp_error(
            span,
            "math.add expected two numbers as its arguments");
        return nullptr;
    }

    return std::make_shared<number>(span, fst->value + snd->value);
}

size_t sub::arity() const noexcept
{
    return 2;
}

object_ptr
sub::call(interpreter& interp, span span, std::vector<object_ptr> args) noexcept
{
    auto fst = std::dynamic_pointer_cast<number>(args[0]);
    auto snd = std::dynamic_pointer_cast<number>(args[1]);

    if (!fst || !snd)
    {
        interp.interp_error(
            span,
            "math.sub expected two numbers as its arguments");
        return nullptr;
    }

    return std::make_shared<number>(span, fst->value - snd->value);
}

size_t mult::arity() const noexcept
{
    return 2;
}

object_ptr mult::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    auto fst = std::dynamic_pointer_cast<number>(args[0]);
    auto snd = std::dynamic_pointer_cast<number>(args[1]);

    if (!fst || !snd)
    {
        interp.interp_error(
            span,
            "math.mult expected two numbers as its arguments");
        return nullptr;
    }

    return std::make_shared<number>(span, fst->value * snd->value);
}

size_t div::arity() const noexcept
{
    return 2;
}

object_ptr
div::call(interpreter& interp, span span, std::vector<object_ptr> args) noexcept
{
    auto fst = std::dynamic_pointer_cast<number>(args[0]);
    auto snd = std::dynamic_pointer_cast<number>(args[1]);

    if (!fst || !snd)
    {
        interp.interp_error(
            span,
            "math.div expected two numbers as its arguments");
        return nullptr;
    }

    if (snd->value == 0.0)
    {
        return std::make_shared<unit>(span);
    }

    return std::make_shared<number>(span, fst->value / snd->value);
}

}
