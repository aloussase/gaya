#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::core
{

struct typeof_ final : public builtin_function
{
    typeof_()
        : builtin_function { "typeof" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

struct assert_ final : public builtin_function
{
    assert_()
        : builtin_function { "assert" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

}
