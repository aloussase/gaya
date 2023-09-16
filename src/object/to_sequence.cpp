#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

object to_sequence(object o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    {
        return create_number_sequence(o.span, AS_NUMBER(o));
    }
    case object_type_sequence:
    {
        return o;
    }
    case object_type_string:
    {
        return create_string_sequence(o.span, AS_STRING(o));
    }
    case object_type_array:
    {
        return create_array_sequence(o.span, AS_ARRAY(o));
    }
    case object_type_unit:
    case object_type_function:
    case object_type_builtin_function:
    {
        assert(0 && "Should not happen");
    }
    }

    assert(0 && "unhandled case in to_sequence");
}

}
