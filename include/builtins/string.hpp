#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::string
{

/**
 * Return the length of the string.
 */
gaya::eval::object::object
length(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return the concatenation of the first string onto the second one.
 */
gaya::eval::object::object
concat(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Convert a string to a number. Unit is returned if the conversion failed.
 */
gaya::eval::object::object
tonumber(interpreter&, span, const std::vector<object>&) noexcept;

}
