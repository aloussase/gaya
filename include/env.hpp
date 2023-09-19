#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <robin_hood.h>

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

    key(identifier_kind k, const std::string& ident)
        : kind { k }
    {
        hash = robin_hood::hash<std::string> {}(ident);
    }

    key(const std::string& ident)
        : key { identifier_kind::global, ident }
    {
    }

    [[nodiscard]] bool is_assignment_target() const noexcept;

    [[nodiscard]] static key global(const std::string&) noexcept;
    [[nodiscard]] static key local(const std::string&) noexcept;
    [[nodiscard]] static key param(const std::string&) noexcept;

    auto operator==(const key& other) const -> bool
    {
        return hash == other.hash;
    }

    identifier_kind kind;
    size_t hash;
};
}

namespace robin_hood
{

template <>
struct hash<gaya::eval::key>
{
    size_t operator()(const gaya::eval::key& k) const noexcept
    {
        return k.hash;
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
    using bindings   = robin_hood::unordered_flat_map<key, object::object>;

    explicit env(parent_ptr p = nullptr);

    /// Set a binding in the environment.
    void set(key&&, value_type) noexcept;

    /**
     * Get an object from this environment, recursively traversing the
     * environment chain.
     */
    [[nodiscard]] object::object get(const key_type&) const noexcept;

    /**
     * Get an identifier at the specified depth in the environment chain.
     */
    [[nodiscard]] object::object get_at(const key_type&, size_t) const noexcept;

    /*
     * Update a value at the specified depth in the environment.
     */
    [[nodiscard]] bool update_at(const key_type&, value_type, size_t) noexcept;

    /**
     * Return the underlying bindings map.
     */
    [[nodiscard]] const bindings& get_bindings() const noexcept;
    [[nodiscard]] bindings& get_bindings() noexcept;

    /**
     * Get this env's parent.
     * @return The env's parent or nullptr if it doesn't have one.
     */
    [[nodiscard]] const parent_ptr parent() const noexcept;

private:
    [[nodiscard]] const env& nth_parent(size_t) const noexcept;
    [[nodiscard]] env& nth_parent(size_t) noexcept;

    bindings _bindings;
    parent_ptr _parent = nullptr;
};

}
