#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::dict
{

/**
 * Return the number of items in the provided dictionary.
 * @param dict <dictionary> The dictionary.
 */
gaya::eval::object::object
length(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Insert a key-value pair in the provided dictionary.
 * This mutates the original dictionary.
 *
 * @param dict <dictionary> The dictionary in which to do the insertion.
 * @param key <object> The key.
 * @param value <object> The value.
 * @return The provided dictionary.
 */
gaya::eval::object::object
set(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Remove the specified key-value pair from the dictionary.
 * This mutates the original dictionary.
 *
 * NOTE: Might want to return the dict instead.
 *
 * @param dict <dictionary> The dictionary from which to remove the pair.
 * @param key <object> The key to remove.
 * @return The removed value.
 */
gaya::eval::object::object
remove(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Returns a truthy value if the specified key has a corresponding value in the
 * provided dictionary, unit otherwise.
 * @param dict <dictionary> The dictionary to test.
 * @param key <object> The key of interest.
 */
gaya::eval::object::object
contains(interpreter&, span, const std::vector<object>&) noexcept;

}
