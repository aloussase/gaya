#include <fmt/core.h>
#include <httplib.h>

#include <builtins/aoc.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::aoc
{

// TODO: Cache the response from the server.

gaya::eval::object::object
get_input(interpreter& interp, span span, const std::vector<object>& args)
{
    auto day    = args[0];
    auto year   = args[1];
    auto cookie = args[2];

    if (!IS_NUMBER(day) || !IS_NUMBER(year))
    {
        interp.interp_error(span, "Expected day and year to be numbers");
        return invalid;
    }

    if (!IS_STRING(cookie))
    {
        interp.interp_error(span, "Expected the cookie to be a string");
        return invalid;
    }

    auto host = "https://adventofcode.com";
    auto path
        = fmt::format("/{}/day/{}/input", AS_NUMBER(year), AS_NUMBER(day));

    httplib::Client client { host };
    httplib::Headers headers {
        { "Cookie", fmt::format("session={}", AS_STRING(cookie)) }
    };

    auto response = client.Get(path, headers);
    if (!response || response->status != 200) return create_unit(span);

    return create_string(interp, span, response->body);
}

}
