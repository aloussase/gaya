#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::array
{

/* TODO: Document these. */

/**
 * @return The length of the array.
 */
gaya::eval::object::object
length(interpreter&, span, std::vector<object>) noexcept;

/**
 * Append the elements of the second array to the first one.
 */
gaya::eval::object::object
concat(interpreter&, span, std::vector<object>) noexcept;

/**
 * Append an element to the end of the array.
 */
gaya::eval::object::object
push(interpreter&, span, std::vector<object>) noexcept;

/**
 * Remove and return the last element of the array.
 */
gaya::eval::object::object
pop(interpreter&, span, std::vector<object>) noexcept;

}
