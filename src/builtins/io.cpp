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
    fmt::println("{}", args.front()->to_string());
    return std::make_shared<unit>(span);
}

}
