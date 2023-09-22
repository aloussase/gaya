#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::math
{

/**
 * Computes the floor of the given number.
 * @param n <number> The number for which to compute the floor.
 */
gaya::eval::object::object
floor(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Computes the ceiling of the given number.
 * @param n <number> The number for which to compute the ceiling.
 */
gaya::eval::object::object
ceil(interpreter&, span, const std::vector<object>&) noexcept;

}
