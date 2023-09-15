#include <fmt/core.h>

#include <builtins/sequence.hpp>
#include <eval.hpp>
#include <sequence.hpp>

namespace gaya::eval::object::builtin::sequence
{

/* seq.next */

size_t next::arity() const noexcept
{
    return 1;
}

object_ptr next::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    if (args[0]->typeof_() != "sequence")
    {
        interp.interp_error(
            span,
            fmt::format("Expected {} to be sequence", args[0]->typeof_()));
        return nullptr;
    }

    auto seq = std::static_pointer_cast<gaya::eval::object::sequence>(args[0]);
    return seq->next();
}

/* seq.map */

size_t map::arity() const noexcept
{
    return 2;
}

object_ptr
map::call(interpreter& interp, span span, std::vector<object_ptr> args) noexcept
{
    auto seq  = args[0];
    auto func = args[1];

    if (!seq->is_sequence())
    {
        interp.interp_error(
            span,
            fmt::format("Expected {} to be a sequence", seq->to_string()));
        return nullptr;
    }

    if (!func->is_callable())
    {
        interp.interp_error(
            span,
            fmt::format("Expected {} to be a callabel", func->to_string()));
        return nullptr;
    }

    return std::make_shared<mapper_sequence>(
        interp,
        span,
        seq->to_sequence(),
        std::dynamic_pointer_cast<callable>(func));
}

/* seq.make */

size_t make::arity() const noexcept
{
    return 1;
}

object_ptr make::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    auto next = args[0];

    if (!next->is_callable())
    {
        interp.interp_error(
            span,
            fmt::format("Expected {} to callable", next->to_string()));
        return nullptr;
    }

    return std::make_shared<user_defined_sequence>(
        span,
        interp,
        std::dynamic_pointer_cast<callable>(next));
}

}
