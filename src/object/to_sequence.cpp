#include <fmt/core.h>
#include <nanbox.h>

#include <object.hpp>

namespace gaya::eval::object
{

object to_sequence(interpreter& interp, object& o) noexcept
{
    switch (o.type)
    {
    case object_type_unit:
    {
        return o;
    }
    case object_type_number:
    {
        return create_number_sequence(interp, o.span, AS_NUMBER(o));
    }
    case object_type_sequence:
    {
        return o;
    }
    case object_type_string:
    {
        return create_string_sequence(interp, o.span, AS_STRING(o));
    }
    case object_type_array:
    {
        return create_array_sequence(interp, o.span, AS_ARRAY(o));
    }
    case object_type_dictionary:
    {
        return create_dict_sequence(interp, o.span, AS_DICT(o));
    }
    case object_type_function:
    case object_type_builtin_function:
    case object_type_foreign_function:
    case object_type_invalid:
    {
        assert(0 && "Should not happen");
    }
    }

    assert(0 && "unhandled case in to_sequence");
}

}
