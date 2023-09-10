#include <fmt/core.h>

#include <builtins/io.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object::builtin::io {

size_t println::arity() const noexcept
{
  return 1;
}

object_ptr println::call(interpreter& interp, std::vector<object_ptr> args) noexcept
{
  for (const auto& arg : args) {
    fmt::println("{}", arg->to_string());
  }

  return std::make_shared<unit>(interp.synthetize_span());
}

}
