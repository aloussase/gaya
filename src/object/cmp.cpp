#include <nanbox.h>

#include <object.hpp>

namespace gaya::eval::object
{

std::optional<int> cmp(object o1, object o2) noexcept
{
    if (o1.type != o2.type) return std::nullopt;

    switch (o1.type)
    {
    case object_type_number:
    {
        auto n1 = AS_NUMBER(o1);
        auto n2 = AS_NUMBER(o2);
        return (n1 < n2) ? -1 : ((n1 == n2) ? 0 : 1);
    }
    case object_type_unit:
    {
        return 0;
    }
    case object_type_string:
    {
        auto cmp = AS_STRING(o1) <=> AS_STRING(o2);
        return (cmp < 0) ? -1 : ((cmp == 0) ? 0 : 1);
    }
    case object_type_array:
    case object_type_function:
    case object_type_builtin_function:
    case object_type_sequence:
    {
        assert(0 && "Should not happen");
    }
    }

    assert(0 && "unhandled case in cmp");
}

}
