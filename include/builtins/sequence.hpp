#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::sequence
{

/* TODO: Document these. */

gaya::eval::object::object
next(interpreter&, span, const std::vector<object>&) noexcept;

gaya::eval::object::object
make(interpreter&, span, const std::vector<object>&) noexcept;

}
