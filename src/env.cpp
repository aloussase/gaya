#include "env.hpp"

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

env::env(parent_ptr p)
    : _parent { p }
{
}

void env::set(const key& k, object::object_ptr v) noexcept
{
    _bindings.insert_or_assign(k, v);
}

object::object_ptr env::get(const std::string& k) const noexcept
{
    // NOTE: We don't really care about the metadata here.
    return get(key::global(k));
}

object::object_ptr env::get(const key& k) const noexcept
{
    if (_bindings.contains(k))
    {
        return _bindings.at(k);
    }

    if (_parent != nullptr)
    {
        return _parent->get(k);
    }

    return nullptr;
}

bool env::update_at(const std::string& k, object::object_ptr new_val) noexcept
{
    return update_at(key::local(k), new_val);
}

bool env::update_at(const key& k, object::object_ptr new_val) noexcept
{
    if (auto it = _bindings.find(k); it != _bindings.end())
    {
        if (auto k_ = it->first; k_.is_assignment_target())
        {
            _bindings.at(k) = new_val;
            return true;
        }
    }

    if (_parent != nullptr)
    {
        return _parent->update_at(k, new_val);
    }

    return false;
}

bool env::can_assign_to(const std::string& ident) noexcept
{
    if (auto it = _bindings.find(key::local(ident)); it != _bindings.end())
    {
        return it->first.is_assignment_target();
    }

    if (_parent != nullptr)
    {
        return _parent->can_assign_to(ident);
    }

    return false;
}

}
