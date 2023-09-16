#include <object.hpp>

namespace gaya::eval::object
{

std::string typeof_(object o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    {
        return "number";
    }
    case object_type_unit:
    {
        return "unit";
    }
    case object_type_string:
    {
        return "string";
    }
    case object_type_array:
    {
        return "array";
    }
    case object_type_function:
    case object_type_builtin_function:
    {
        return "function";
    }
    case object_type_sequence:
    {
        return "sequence";
    }
    }

    assert(0 && "unhandled case in typeof_");
}

}
