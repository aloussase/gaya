#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

bool is_comparable(const object& o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    case object_type_string:
    case object_type_unit:
    {
        return true;
    }
    case object_type_array:
    case object_type_dictionary:
    case object_type_function:
    case object_type_builtin_function:
    case object_type_foreign_function:
    case object_type_sequence:
    case object_type_struct:
    case object_type_invalid:
    {
        return false;
    }
    }

    assert(0 && "unhandled case in is_comparable");
}

}
