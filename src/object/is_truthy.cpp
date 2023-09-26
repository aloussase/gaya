#include <nanbox.h>

#include <object.hpp>

namespace gaya::eval::object
{

bool is_truthy(const object& o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    {
        return nanbox_to_double(o.box) != 0.0;
    }
    case object_type_unit:
    {
        return false;
    }
    case object_type_string:
    {
        return !AS_STRING(o).empty();
    }
    case object_type_array:
    {
        return !AS_ARRAY(o).empty();
    }
    case object_type_dictionary:
    {
        return !AS_DICT(o).empty();
    }
    case object_type_function:
    case object_type_builtin_function:
    case object_type_foreign_function:
    {
        return true;
    }
    case object_type_sequence:
    {
        return false;
    }

    case object_type_invalid:
    {
        assert(0 && "is_truthy of invalid object");
    }
    }

    assert(0 && "uhandled case in is_truthy");
}

}
