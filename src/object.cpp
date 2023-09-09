#include <fmt/core.h>

#include <ast.hpp>
#include <object.hpp>

namespace gaya::eval::object {

function::function(span s, std::vector<ast::identifier> p, ast::expression_ptr b)
    : _span { s }
    , params { std::move(p) }
    , arity { params.size() }
    , body { std::move(b) }
{
}

std::string function::to_string() const noexcept
{
  return fmt::format("<function-{}>", arity);
}

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
