#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::core
{

/* TODO: Document these. */

gaya::eval::object::object
typeof_(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::object
assert_(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::object
tostring(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::object
issequence(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::object
tosequence(interpreter&, span, std::vector<object>) noexcept;

}
