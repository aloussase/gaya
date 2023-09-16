#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::array
{

/* TODO: Document these. */

gaya::eval::object::maybe_object
length(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
concat(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
push(interpreter&, span, std::vector<object>) noexcept;

}
