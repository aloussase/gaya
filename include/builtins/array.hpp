#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::array
{

/**
 * Return the length of an array.
 * @param a <array> The array.
 */
gaya::eval::object::object
length(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Append the elements of the second array to the end first one.
 * This mutates the first array.
 * @param a1 <array> The array onto which to append elements.
 * @param a2 <array> The array from which to extract elements.
 */
gaya::eval::object::object
concat(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Append an element to the end of the array.
 * This mutates the original array.
 * @param a <array> The array onto which to push the element.
 * @param o <object> The element to append.
 */
gaya::eval::object::object
push(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Remove and return the last element of the array.
 * This mutates the original array.
 * @param a <array> The array from which to remove the last element.
 */
gaya::eval::object::object
pop(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Sort the provided array in place.
 * If the elements of the array are not comparable, an interpreter error takes
 * place.
 * @param a <array> The array to sort.
 */
gaya::eval::object::object
sort(interpreter&, span, const std::vector<object>&) noexcept;

}
