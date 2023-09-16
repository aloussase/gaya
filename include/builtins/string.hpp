#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::string
{

/* TODO: Document these. */

gaya::eval::object::maybe_object
length(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
concat(interpreter&, span, std::vector<object>) noexcept;

}
