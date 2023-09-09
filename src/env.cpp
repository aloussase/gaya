#include "env.hpp"

namespace eval {

env::env(parent_ptr p)
    : _parent { p }
{
}

void env::set(const std::string& k, object::object_ptr v) noexcept
{
  _bindings.insert_or_assign(k, v);
}

object::object_ptr env::get(const std::string& k) const noexcept
{
  if (_bindings.contains(k)) {
    return _bindings.at(k);
  }

  if (_parent != nullptr) {
    return _parent->get(k);
  }

  return nullptr;
}

}
