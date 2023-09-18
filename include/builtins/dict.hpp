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

}
