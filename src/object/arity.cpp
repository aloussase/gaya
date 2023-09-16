#include <cassert>

#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

size_t arity(object o) noexcept
{
    switch (o.type)
    {

    case object_type_string:
    case object_type_array:
    {
        return 1;
    }
    case object_type_function:
    {
        return AS_FUNCTION(o).arity;
    }
    case object_type_builtin_function:
    {
        return AS_BUILTIN_FUNCTION(o).arity;
    }
    case object_type_number:
    case object_type_unit:
    case object_type_sequence:
    case object_type_invalid:
    {
        assert(0 && "Should not happen");
    }
    }

    assert(0 && "unhandled case in arity");
}

}
