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

/**
 * Return the index in the string where the provided pattern starts, or unit if
 * the pattern is not contained in the string.
 */
gaya::eval::object::object
index(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return a substring of the given string.
 *
 * If finish < start or start < 0 or finish > string.length(s), unit
 * is returned instead.*
 *
 * @param s <string> The string from which to extract the substring.
 * @param start <number> The start of the substring.
 * @param finish <number> The end of the substring.
 */
gaya::eval::object::object
substring(interpreter&, span, const std::vector<object>&) noexcept;

}
