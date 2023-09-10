#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::math
{

struct add final : public builtin_function
{
    add()
        : builtin_function { "math.add" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

struct sub final : public builtin_function
{
    sub()
        : builtin_function { "math.sub" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

struct mult final : public builtin_function
{
    mult()
        : builtin_function { "math.mult" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

struct div final : public builtin_function
{
    div()
        : builtin_function { "math.div" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

}
