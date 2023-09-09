#include <fmt/core.h>

#include <builtins/io.hpp>

namespace gaya::eval::object::builtin::io {

size_t println::arity() const noexcept
{
  return 1;
}

object_ptr println::call(interpreter&, std::vector<object_ptr> args) noexcept
{
  for (const auto& arg : args) {
    fmt::println("{}", arg->to_string());
  }
  // TODO: Return unit
  // TODO: Add a get_span method to interpreter to syntehtize a span.
  return nullptr;
}

}
