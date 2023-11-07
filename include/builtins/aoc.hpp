#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::aoc
{

/**
 * Download the input for the given day.
 * @param day <aoc.Day> The day to download the input for.
 * @param cookie <String> The session cookie to use.
 */
gaya::eval::object::object
get_input(interpreter&, span, const std::vector<object>&);

}
