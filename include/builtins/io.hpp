#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

/**
 * Print a line of text to standard output.
 * @param line <string> The line to print.
 */
gaya::eval::object::object
println(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Print some text to standard output.
 * @param text <string> The text to print.
 */
gaya::eval::object::object
print(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Read a line of text from standard input. The resulting line will not include
 * the newline.
 */
gaya::eval::object::object
readline(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Read the contents of a file. If the file does not exist, unit is returned.
 * @param filename <string> The name of the file to read.
 */
gaya::eval::object::object
readfile(interpreter&, span, const std::vector<object>&) noexcept;

}
