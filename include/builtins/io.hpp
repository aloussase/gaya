#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

/**
 * Print a line of text to standard output.
 */
gaya::eval::object::object
println(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Print some text to standard output.
 */
gaya::eval::object::object
print(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Read a line of text from standard input. The resulting line will not include
 * the newline.
 */
gaya::eval::object::object
readline(interpreter&, span, const std::vector<object>&) noexcept;

}
