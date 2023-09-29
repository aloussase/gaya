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

std::string
array_to_string(interpreter& interp, const std::vector<object>& elems)
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

std::string dict_to_string(
    interpreter& interp,
    const robin_hood::unordered_map<object, object>& dict)
{
    if (dict.empty())
    {
        return "(->)";
    }

    std::stringstream ss;
    ss << "(";
    std::size_t i = 0;
    for (auto& it : dict)
    {
        ss << to_string(interp, it.first) << " -> "
           << to_string(interp, it.second);

        if (i < dict.size() - 1)
        {
            ss << ", ";
        }

        i += 1;
    }
    ss << ")";
    return ss.str();
}

std::string sequence_to_string(interpreter& interp, sequence& seq) noexcept
{
    std::stringstream ss;

    ss << "(";

    auto o = next(interp, seq);
    if (gaya::eval::object::is_valid(o) && o.type != object_type_unit)
    {
        ss << to_string(interp, o);

        for (;;)
        {
            auto o = next(interp, seq);
            if (!gaya::eval::object::is_valid(o) || o.type == object_type_unit)
                break;

            ss << ", " << to_string(interp, o);
        }
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
    case object_type_dictionary:
    {
        return dict_to_string(interp, AS_DICT(o));
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
    case object_type_foreign_function:
    {
        auto func_name = AS_FOREIGN_FUNCTION(o).funcname;
        return fmt::format("<foreign-function: {}>", func_name);
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
