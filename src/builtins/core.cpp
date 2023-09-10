#include <builtins/core.hpp>

namespace gaya::eval::object::builtin::core
{

size_t typeof_::arity() const noexcept
{
    return 1;
}

object_ptr
typeof_::call(interpreter&, span span, std::vector<object_ptr> args) noexcept
{
    auto& o   = args[0];
    auto type = o->typeof_();
    return std::make_shared<string>(span, type);
}

}
