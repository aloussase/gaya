#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::sequence
{

/* TODO: Document these. */

gaya::eval::object::maybe_object
next(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
make(interpreter&, span, std::vector<object>) noexcept;

}
