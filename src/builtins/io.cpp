#include <iostream>

#include <fmt/core.h>

#include <builtins/io.hpp>
#include <eval.hpp>
#include <file_reader.hpp>
#include <object.hpp>

namespace gaya::eval::object::builtin::io
{

gaya::eval::object::object println(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    if (IS_STRING(args[0]))
    {
        fmt::println("{}", AS_STRING(args[0]));
    }
    else
    {
        auto line = to_string(interp, args[0]);
        fmt::println("{}", line);
    }

    return create_unit(span);
}

gaya::eval::object::object
print(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    if (IS_STRING(args[0]))
    {
        fmt::print("{}", AS_STRING(args[0]));
    }
    else
    {
        auto line = to_string(interp, args[0]);
        fmt::print("{}", line);
    }
    return create_unit(span);
}

gaya::eval::object::object
readline(interpreter& interp, span span, const std::vector<object>&) noexcept
{
    std::string line;
    std::getline(std::cin, line);
    return create_string(interp, span, std::move(line));
}

gaya::eval::object::object readfile(
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    auto& filename = args[0];
    if (!IS_STRING(filename))
    {
        interp.interp_error(span, "Expected first argument to be a string");
        return invalid;
    }

    file_reader reader { AS_STRING(filename) };
    if (!reader)
    {
        return create_unit(span);
    }

    /*
     * NOTE: We don't free contents because the returned string will take
     *       ownership of it.
     */
    char* contents = reader.slurp();
    assert(contents);
    std::string contents_as_string(contents);

    return create_string(interp, span, contents);
}
}
