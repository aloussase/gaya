#include <fmt/core.h>

#include "env.hpp"
#include <object.hpp>

namespace gaya::eval
{

bool key::is_assignment_target() const noexcept
{
    return kind == identifier_kind::local;
}

key key::global(const std::string& ident) noexcept
{
    return key { identifier_kind::global, ident };
}

key key::local(const std::string& ident) noexcept
{
    return key { identifier_kind::local, ident };
}

key key::param(const std::string& ident) noexcept
{
    return key { identifier_kind::param, ident };
}

const env::bindings& env::get_bindings() const noexcept
{
    return _bindings;
}

env::bindings& env::get_bindings() noexcept
{
    return _bindings;
}

const env::parent_ptr env::parent() const noexcept
{
    return _parent;
}

env::env(parent_ptr p)
    : _parent { p }
{
}

env env::deep_copy(interpreter& interp, span span) const noexcept
{
    env new_env {
        _parent ? std::make_shared<env>(_parent->deep_copy(interp, span))
                : _parent,
    };

    for (auto [k, o] : _bindings)
    {
        if (IS_SEQUENCE(o))
        {
            new_env.set(
                std::move(k),
                object::copy_sequence(interp, span, AS_SEQUENCE(o)));
        }
        else
        {
            new_env.set(std::move(k), o);
        }
    }

    return new_env;
}

void env::set(key&& k, value_type v) noexcept
{
    _bindings.insert_or_assign(std::move(k), v);
}

object::object env::get(const key_type& k) const noexcept
{
    if (auto it = _bindings.find(k); it != _bindings.end())
    {
        return it->second;
    }

    if (_parent != nullptr)
    {
        return _parent->get(k);
    }

    return object::invalid;
}

const env& env::nth_parent(size_t n) const noexcept
{
    auto* environment = this;

    for (size_t i = 0; i < n; i++)
    {
        assert(environment);
        environment = environment->parent().get();
    }

    assert(environment);
    return *environment;
}

env& env::nth_parent(size_t n) noexcept
{
    auto* environment = this;

    for (size_t i = 0; i < n; i++)
    {
        assert(environment);
        environment = environment->parent().get();
    }

    assert(environment);
    return *environment;
}

object::object env::get_at(const key_type& k, size_t depth) const noexcept
{
    const auto& environment = nth_parent(depth);
    const auto& bindings    = environment.get_bindings();

    if (auto it = bindings.find(k); it != bindings.end())
    {
        return it->second;
    }

    return object::invalid;
}

bool env::update_at(const key_type& k, value_type v, size_t depth) noexcept
{
    auto& environment = nth_parent(depth);
    auto& bindings    = environment.get_bindings();

    if (auto it = bindings.find(k); it != bindings.end())
    {
        it->second = v;
        return true;
    }

    return false;
}

}
