#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::array
{

/* TODO: Document these. */

gaya::eval::object::object
length(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::object
concat(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::object
push(interpreter&, span, std::vector<object>) noexcept;

}
