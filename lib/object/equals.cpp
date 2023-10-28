#include <nanbox.h>
#include <object.hpp>

namespace gaya::eval::object
{

bool struct_equals(StructObject& s1, StructObject& s2)
{
    if (s1.name != s2.name) return false;
    if (s1.fields.size() != s2.fields.size()) return false;

    for (size_t i = 0; i < s1.fields.size(); i++)
    {
        auto f1 = s1.fields[i];
        auto f2 = s2.fields[i];

        if (f1.identifier != f2.identifier) return false;
        if (!equals(f1.value, f2.value)) return false;
    }

    return true;
}

bool array_equals(
    const std::vector<object>& xs,
    const std::vector<object>& ys) noexcept
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

bool dict_equals(
    const robin_hood::unordered_map<object, object>& d1,
    const robin_hood::unordered_map<object, object>& d2) noexcept
{
    for (auto it1 : d1)
    {
        if (auto it2 = d2.find(it1.first); it2 != d2.end())
        {
            if (!equals(it1.second, it2->second))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool equals(const object& o1, const object& o2) noexcept
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
    case object_type_dictionary:
    {
        return dict_equals(AS_DICT(o1), AS_DICT(o2));
    }
    case object_type_struct:
    {
        return struct_equals(AS_STRUCT(o1), AS_STRUCT(o2));
    }
    case object_type_enum:
    {
        return AS_ENUM(o1).variant == AS_ENUM(o2).variant;
    }
    case object_type_function:
    case object_type_builtin_function:
    case object_type_sequence:
    case object_type_invalid:
    {
        return false;
    }
    }

    assert(0 && "unhandled case in equals");
}

}
