#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

gaya::eval::object::object
println(interpreter&, span, std::vector<object>) noexcept;

}
