#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::sequence
{

struct hasnext final : public builtin_function
{
    hasnext()
        : builtin_function { "seq.hasnext" }
    {
    }

    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

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

}
