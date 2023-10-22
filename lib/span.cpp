#include <cassert>
#include <cstring>

#include "span.hpp"

span span::invalid = span { nullptr, 0, nullptr, nullptr };

std::string span::to_string() const noexcept
{
    return { _start, _end };
}
