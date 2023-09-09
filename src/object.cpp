#include <fmt/core.h>

#include <object.hpp>

namespace object {

std::string number::to_string() const noexcept
{
  return std::to_string(value);
}

std::string string::to_string() const noexcept
{
  return value;
}

std::string unit::to_string() const noexcept
{
  return "unit";
}

}
