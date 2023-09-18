#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::dict
{

/**
 * Return the number of items in the provided dictionary.
 */
gaya::eval::object::object
length(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Insert a key-value pair in the provided dictionary.
 * This mutates the original dictionary.
 *
 * @return The provided dictionary.
 */
gaya::eval::object::object
insert(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Remove the specified key-value pair from the dictionary.
 * This mutates the original dictionary.
 *
 * @return The removed value.
 * NOTE: Might want to return the dict instead.
 */
gaya::eval::object::object
remove(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Returns a truthy value if the specified key has a
 * corresponding value in the provided dictionary, unit
 * otherwise.
 */
gaya::eval::object::object
contains(interpreter&, span, const std::vector<object>&) noexcept;

}
