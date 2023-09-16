#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <object.hpp>

namespace gaya::eval
{

enum class identifier_kind {
    param,
    global,
    local,
};

struct key
{
    key(identifier_kind k, const std::string& i)
        : kind { k }
        , ident { i }
    {
    }

    key(const char* i)
        : kind { identifier_kind::global }
        , ident { i }
    {
    }

    [[nodiscard]] bool is_assignment_target() const noexcept;

    [[nodiscard]] static key global(const std::string&) noexcept;
    [[nodiscard]] static key local(const std::string&) noexcept;
    [[nodiscard]] static key param(const std::string&) noexcept;

    identifier_kind kind;
    std::string ident;
};

}

namespace std
{

template <>
struct hash<gaya::eval::key>
{
    size_t operator()(const gaya::eval::key& k) const noexcept
    {
        return hash<string> {}(k.ident);
    }
};

template <>
struct equal_to<gaya::eval::key>
{
    bool
    operator()(const gaya::eval::key& lhs, const gaya::eval::key& rhs) const
    {
        return equal_to<string> {}(lhs.ident, rhs.ident);
    }
};

}

namespace gaya::eval
{

class env final
{
public:
    using parent_ptr = std::shared_ptr<env>;
    using value_type = object::object;
    using key_type   = key;
    using bindings   = std::unordered_map<key_type, value_type>;

    explicit env(parent_ptr p = nullptr);

    /// Set a binding in the environment.
    void set(const key&, value_type) noexcept;

    /// Get a binding from the environment.
    [[nodiscard]] object::maybe_object get(const key_type&) const noexcept;
    [[nodiscard]] object::maybe_object get(const std::string&) const noexcept;

    /// Try to update a value in the environment and return false if the value
    /// was not found.
    [[nodiscard]] bool update_at(const key_type&, value_type) noexcept;
    [[nodiscard]] bool update_at(const std::string&, value_type) noexcept;

    /// Check whether the provided identifier is a valid assignment target.
    [[nodiscard]] bool can_assign_to(const std::string&) noexcept;

    /**
     * Return the underlying bindings map.
     */
    [[nodiscard]] const bindings& get_bindings() const noexcept;

    /**
     * Get this env's parent.
     * @return The env's parent or nullptr if it doesn't have one.
     */
    [[nodiscard]] const parent_ptr parent() const noexcept;

private:
    bindings _bindings;
    parent_ptr _parent = nullptr;
};

}
