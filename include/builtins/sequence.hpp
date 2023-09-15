#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::sequence
{

struct next final : public builtin_function
{
    next()
        : builtin_function { "seq.next" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

struct map final : public builtin_function
{
    map()
        : builtin_function { "seq.map" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

struct make final : public builtin_function
{
    make()
        : builtin_function { "seq.make" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

}
