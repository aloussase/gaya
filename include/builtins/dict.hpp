#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::dict
{

gaya::eval::object::object
length(interpreter&, span, const std::vector<object>&) noexcept;

}
