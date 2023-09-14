#include <fmt/core.h>

#include <builtins/io.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

size_t println::arity() const noexcept
{
    return 1;
}

object_ptr
println::call(interpreter&, span span, std::vector<object_ptr> args) noexcept
{
    auto x = args[0];
    auto s = x->to_string();

    if (x->typeof_() == "string")
    {
        s.erase(s.cbegin());
        s.erase(s.cend() - 1);
    }

    fmt::println("{}", s);

    return std::make_shared<unit>(span);
}

}
