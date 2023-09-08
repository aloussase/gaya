#include <cstring>

#include "span.hpp"

std::string span::to_string() const noexcept
{
  return { _start, _end };
}
