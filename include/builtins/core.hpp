#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::core
{

/* TODO: Document these. */

gaya::eval::object::maybe_object
typeof_(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
assert_(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
tostring(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
issequence(interpreter&, span, std::vector<object>) noexcept;

gaya::eval::object::maybe_object
tosequence(interpreter&, span, std::vector<object>) noexcept;

}
