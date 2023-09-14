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

size_t concat::arity() const noexcept
{
    return 2;
}

object_ptr concat::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    if (args[0]->typeof_() != "array" || args[1]->typeof_() != "array")
    {
        interp.interp_error(
            span,
            fmt::format(
                "Expected {} and {} to be both arrays",
                args[0]->typeof_(),
                args[1]->typeof_()));
        return nullptr;
    }

    auto a1 = std::static_pointer_cast<gaya::eval::object::array>(args[0]);
    auto a2 = std::static_pointer_cast<gaya::eval::object::array>(args[1]);

    std::vector<object_ptr> new_elems;

    new_elems.insert(new_elems.cbegin(), a1->elems.begin(), a1->elems.end());
    new_elems.insert(
        new_elems.cbegin() + std::distance(a1->elems.begin(), a1->elems.end()),
        a2->elems.begin(),
        a2->elems.end());

    return std::make_shared<gaya::eval::object::array>(span, new_elems);
}

size_t push::arity() const noexcept
{
    return 2;
}

object_ptr push::call(
    interpreter& interp,
    span span,
    std::vector<object_ptr> args) noexcept
{
    if (args[0]->typeof_() != "array")
    {
        interp.interp_error(span, "Expected first argument to be an array");
        return nullptr;
    }

    auto a = std::static_pointer_cast<gaya::eval::object::array>(args[0]);

    auto new_elems = std::vector { a->elems };
    new_elems.push_back(args[1]);

    return std::make_shared<gaya::eval::object::array>(span, new_elems);
}

}
