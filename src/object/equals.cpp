#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

bool array_equals(std::vector<object> xs, std::vector<object> ys) noexcept
{
    if (xs.size() != ys.size()) return false;

    for (size_t i = 0; i < xs.size(); i++)
    {
        if (!equals(xs[i], ys[i]))
        {
            return false;
        }
    }

    return true;
}

bool equals(object o1, object o2) noexcept
{
    if (o1.type != o2.type) return false;

    switch (o1.type)
    {
    case object_type_number:
    {
        return AS_NUMBER(o1) == AS_NUMBER(o2);
    }
    case object_type_unit:
    {
        return true;
    }
    case object_type_string:
    {
        return AS_STRING(o1) == AS_STRING(o2);
    }
    case object_type_array:
    {
        return array_equals(AS_ARRAY(o1), AS_ARRAY(o2));
    }
    case object_type_function:
    case object_type_builtin_function:
    case object_type_sequence:
    {
        return false;
    }
    }

    assert(0 && "unhandled case in equals");
}

}
