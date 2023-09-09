#pragma once

#include <memory>
#include <unordered_map>

#include <object.hpp>

namespace eval {

class env final {
public:
  using parent_ptr = std::shared_ptr<env>;
  using bindings   = std::unordered_map<std::string, object::object_ptr>;

  env(parent_ptr p = nullptr);

  /// Set a binding in the environment.
  void set(const std::string&, object::object_ptr) noexcept;

  /// Get a binding from the environment.
  [[nodiscard]] object::object_ptr get(const std::string&) const noexcept;

private:
  bindings _bindings;
  parent_ptr _parent = nullptr;
};

}
