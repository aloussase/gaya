#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

gaya::eval::object::maybe_object
println(interpreter&, span, std::vector<object>) noexcept;

}
