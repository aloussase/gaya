#include <iostream>

#include <fmt/core.h>

#include <ast.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object {

function::function(span s, std::vector<ast::identifier> p, ast::expression_ptr b, eval::env e)
    : _span { s }
    , params { std::move(p) }
    , _arity { params.size() }
    , body { std::move(b) }
    , closed_over_env { std::move(e) }
{
}

std::string function::to_string() const noexcept
{
  return fmt::format("<function-{}>", _arity);
}

bool function::is_callable() const noexcept
{
  return true;
}

size_t function::arity() const noexcept
{
  return _arity;
}

object_ptr function::call(interpreter& interp, std::vector<object_ptr> args) noexcept
{
  interp.begin_scope(closed_over_env);
  for (size_t i = 0; i < args.size(); i++) {
    auto param_name = params[i];
    auto arg        = args[i];
    interp.define(param_name.value, arg);
  }
  auto ret = body->accept(interp);
  interp.end_scope();
  return ret;
}

std::string builtin_function::to_string() const noexcept
{
  return fmt::format("<native-function: {}>", name);
}

bool builtin_function::is_callable() const noexcept
{
  return true;
}

std::string number::to_string() const noexcept
{
  // NOTE: Maybe we do want full precision here.
  return fmt::format("{:.2f}", value);
}

bool number::is_callable() const noexcept
{
  return false;
}

std::string string::to_string() const noexcept
{
  std::string cpy = value;
  if (cpy.empty()) return cpy;

  cpy.replace(0, 1, "");
  cpy.replace(cpy.size() - 1, 1, "");

  return cpy;
}

bool string::is_callable() const noexcept
{
  return false;
}

std::string unit::to_string() const noexcept
{
  return "unit";
}

bool unit::is_callable() const noexcept
{
  return false;
}

}
