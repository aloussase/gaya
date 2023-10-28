#include <nanbox.h>

#include <object.hpp>

namespace gaya::eval::object
{

bool cmp(const object& o1, const object& o2, int* result) noexcept
{
    if (o1.type != o2.type) return false;

    switch (o1.type)
    {
    case object_type_number:
    {
        auto n1 = AS_NUMBER(o1);
        auto n2 = AS_NUMBER(o2);
        *result = (n1 < n2) ? -1 : ((n1 == n2) ? 0 : 1);
        return true;
    }
    case object_type_unit:
    {
        *result = 0;
        return true;
    }
    case object_type_string:
    {
        auto cmp = AS_STRING(o1) <=> AS_STRING(o2);
        *result  = (cmp < 0) ? -1 : ((cmp == 0) ? 0 : 1);
        return true;
    }
    case object_type_enum:
    {
        auto e1 = AS_ENUM(o1);
        auto e2 = AS_ENUM(o2);
        auto v1 = e1.variants[e1.variant];
        auto v2 = e2.variants[e1.variant];
        *result = (v1 < v2) ? -1 : ((v1 == v2) ? 0 : 1);
        return true;
    }
    case object_type_array:
    case object_type_dictionary:
    case object_type_function:
    case object_type_builtin_function:
    case object_type_sequence:
    case object_type_struct:
    case object_type_invalid:
    {
        assert(0 && "The elements are not comparable!");
        return false;
    }
    }

    assert(0 && "unhandled case in cmp");
}

}
