#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

bool is_comparable(object o) noexcept
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
    case object_type_function:
    case object_type_builtin_function:
    case object_type_sequence:
    {
        return false;
    }
    }
}

}
