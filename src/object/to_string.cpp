#include <cmath>
#include <sstream>

#include <fmt/core.h>
#include <nanbox.h>

#include <object.hpp>

namespace gaya::eval::object
{

std::string number_to_string(double number)
{
    double intval;
    auto has_decimals = std::modf(number, &intval) != 0.0;
    return has_decimals ? fmt::format("{}", number)
                        : fmt::format("{:.0f}", number);
}

std::string array_to_string(interpreter& interp, std::vector<object> elems)
{
    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < elems.size(); i++)
    {
        ss << to_string(interp, elems[i]);
        if (i < elems.size() - 1)
        {
            ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}

std::string sequence_to_string(interpreter& interp, sequence& seq) noexcept
{
    std::stringstream ss;

    ss << "(";

    auto o = next(interp, seq);
    if (gaya::eval::object::is_valid(o))
    {
        ss << to_string(interp, o);
    }

    for (;;)
    {
        auto o = next(interp, seq);
        if (!gaya::eval::object::is_valid(o)) break;

        ss << ", " << to_string(interp, o);
    }

    ss << ")";

    return ss.str();
}

std::string to_string(interpreter& interp, object o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    {
        return number_to_string(AS_NUMBER(o));
    }
    case object_type_unit:
    {
        return "unit";
    }
    case object_type_string:
    {
        return '"' + AS_STRING(o) + '"';
    }
    case object_type_array:
    {
        return array_to_string(interp, AS_ARRAY(o));
    }
    case object_type_function:
    {
        return fmt::format("<function-{}>", AS_FUNCTION(o).arity);
    }
    case object_type_builtin_function:
    {
        auto func_name = AS_BUILTIN_FUNCTION(o).name;
        return fmt::format("<builtin-function: {}>", func_name);
    }
    case object_type_sequence:
    {
        return sequence_to_string(interp, AS_SEQUENCE(o));
    }
    case object_type_invalid:
    {
        assert(0 && "to_string of invalid object");
    }
    }

    assert(0 && "unhandled case in to_string");
}

}
