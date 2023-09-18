#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

bool is_callable(const object& o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    case object_type_unit:
    case object_type_sequence:
    {
        return false;
    }
    case object_type_string:
    case object_type_array:
    case object_type_dictionary:
    case object_type_function:
    case object_type_builtin_function:
    {
        return true;
    }
    case object_type_invalid:
    {
        assert(0 && "is_callable of invalid object");
    }
    }

    assert(0 && "unhandled case in is_callable");
}

}
