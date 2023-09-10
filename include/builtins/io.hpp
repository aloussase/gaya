#pragma once

#include <object.hpp>

namespace gaya::eval::object::builtin::io {

struct println final : public builtin_function {
  println()
      : builtin_function { "io.println" }
  {
  }

  size_t arity() const noexcept override;
  object_ptr call(interpreter&, span, std::vector<object_ptr> args) noexcept override;
};

}
