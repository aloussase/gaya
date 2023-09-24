#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::string
{

/**
 * Return the length of the string.
 * @param s <string> The string for which to calculate the length.
 */
gaya::eval::object::object
length(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return the concatenation of the second string onto the first one.
 * @param s1 <string> The first string.
 * @param s2 <string> The string to tack onto the end of the first one.
 */
gaya::eval::object::object
concat(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Convert a string to a number.
 * Unit is returned if the conversion failed.
 * @param s <string> The string to try to parse as a number.
 */
gaya::eval::object::object
tonumber(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return the index in the string where the provided pattern starts, or unit if
 * the pattern is not contained in the string.
 * @param haystack <string> The string in which to search for the pattern.
 * @param needle <string> The string to search for.
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

/**
 * Return whether a string starts with the given pattern.
 *
 * @param s <string> The string to test.
 * @param pattern <string> The pattern to search for in s.
 * @param pos <number> The position in s from where to start testing.
 */
gaya::eval::object::object
startswith(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return whether a string ends with the given pattern.
 *
 * @param s <string> The string to test.
 * @param pattern <string> The pattern to search for in s.
 */
gaya::eval::object::object
endswith(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return a new string that is the result of trimming the leading and trailing
 * whitespace from the provided string.
 * @param s <string> The string to trim.
 */
gaya::eval::object::object
trim(interpreter&, span, const std::vector<object>&) noexcept;

}
