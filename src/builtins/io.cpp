#include <iostream>

#include <fmt/core.h>

#include <builtins/io.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

gaya::eval::object::object
println(interpreter& interp, span span, std::vector<object> args) noexcept
{
    using namespace gaya::eval::object;

    auto x = args[0];
    auto s = to_string(interp, x);

    if (x.type == object_type_string)
    {
        s.erase(s.cbegin());
        s.erase(s.cend() - 1);
    }

    fmt::println("{}", s);

    return create_unit(span);
}

gaya::eval::object::object
print(interpreter& interp, span span, std::vector<object> args) noexcept
{
    using namespace gaya::eval::object;

    auto x = args[0];
    auto s = to_string(interp, x);

    if (x.type == object_type_string)
    {
        s.erase(s.cbegin());
        s.erase(s.cend() - 1);
    }

    fmt::print("{}", s);

    return create_unit(span);
}

gaya::eval::object::object
readline(interpreter& interp, span span, std::vector<object>) noexcept
{
    std::string line;
    std::getline(std::cin, line);
    return create_string(interp, span, std::move(line));
}

}
