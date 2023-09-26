#include <object.hpp>

namespace gaya::eval::object
{

std::string typeof_(const object& o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    {
        return "Number";
    }
    case object_type_unit:
    {
        return "Unit";
    }
    case object_type_string:
    {
        return "String";
    }
    case object_type_array:
    {
        return "Array";
    }
    case object_type_dictionary:
    {
        return "Dictionary";
    }
    case object_type_function:
    case object_type_builtin_function:
    {
        return "Function";
    }
    case object_type_sequence:
    {
        return "Sequence";
    }
    case object_type_invalid:
    {
        assert(0 && "typeof invalid object");
    }
    }

    assert(0 && "unhandled case in typeof_");
}

}
