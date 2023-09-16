#include <object.hpp>

namespace gaya::eval::object
{

bool is_sequence(object o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    case object_type_sequence:
    case object_type_string:
    case object_type_array:
    {
        return true;
    }
    case object_type_unit:
    case object_type_function:
    case object_type_builtin_function:
    {
        return false;
    }
    case object_type_invalid:
    {
        assert(0 && "is_sequence of invalid object");
    }
    }

    assert(0 && "unhandled case in is_sequence");
}

}
