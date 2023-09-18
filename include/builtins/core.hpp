#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::core
{

/* TODO: Document these. */

gaya::eval::object::object
typeof_(interpreter&, span, const std::vector<object>&) noexcept;

gaya::eval::object::object
assert_(interpreter&, span, const std::vector<object>&) noexcept;

gaya::eval::object::object
tostring(interpreter&, span, const std::vector<object>&) noexcept;

gaya::eval::object::object
issequence(interpreter&, span, const std::vector<object>&) noexcept;

gaya::eval::object::object
tosequence(interpreter&, span, const std::vector<object>&) noexcept;

}
