#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::array
{

struct length final : public builtin_function
{
    length()
        : builtin_function("array.length")
    {
    }

    size_t arity() const noexcept override;

    object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept override;
};

}
