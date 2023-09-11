#pragma once

#include <memory>
#include <unordered_map>

namespace gaya::eval::object
{

struct object;
using object_ptr = std::shared_ptr<object>;

}

namespace gaya::eval
{

class env final
{
  public:
    using parent_ptr = std::shared_ptr<env>;
    using bindings   = std::unordered_map<std::string, object::object_ptr>;

    explicit env(parent_ptr p = nullptr);

    /// Set a binding in the environment.
    void set(const std::string&, object::object_ptr) noexcept;

    /// Get a binding from the environment.
    [[nodiscard]] object::object_ptr get(const std::string&) const noexcept;

    /// Try to update a value in the environment and return false if the value
    /// was not found.
    [[nodiscard]] bool
    update_at(const std::string&, object::object_ptr) noexcept;

  private:
    bindings _bindings;
    parent_ptr _parent = nullptr;
};

}
