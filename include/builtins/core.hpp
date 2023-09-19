#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::core
{

/**
 * Return the type of an object as a string.
 * See src/object/typeof.cpp for the possible types.
 */
gaya::eval::object::object
typeof_(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Assert that something is truthy. If false, an intepreter error occurs with
 * the message that an assertion failed.
 */
gaya::eval::object::object
assert_(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return the string representation of the provided object.
 */
gaya::eval::object::object
tostring(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return whether a given object can act as a sequence.
 */
gaya::eval::object::object
issequence(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Convert the provided object into its sequence representation.
 */
gaya::eval::object::object
tosequence(interpreter&, span, const std::vector<object>&) noexcept;

}
