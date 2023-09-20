#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::sequence
{

/**
 * Return the next element in the sequence, or unit if there is none.
 * @param xs <sequence> The sequence from which to retrieve the next element.
 */
gaya::eval::object::object
next(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Create a new user defined sequence from the provided callable.
 *
 * The given callable should return the next element on each invocation, and
 * unit when it is done.
 * @param cb <function> The function to be invoked to retrieve the next element.
 */
gaya::eval::object::object
make(interpreter&, span, const std::vector<object>&) noexcept;

}
