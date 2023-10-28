#include <functional>

#include <robin_hood.h>

#include <object.hpp>

namespace gaya::eval::object
{

size_t hash(const object& o) noexcept
{
    switch (o.type)
    {
    case object_type_number:
    {
        return robin_hood::hash<double> {}(AS_NUMBER(o));
    }
    case object_type_unit:
    {
        return robin_hood::hash_bytes("unit", sizeof(char) * 4);
    }
    case object_type_string:
    {
        return robin_hood::hash_bytes(
            AS_STRING(o).c_str(),
            sizeof(char) * AS_STRING(o).size());
    }
    case object_type_array:
    {
        auto ary         = AS_ARRAY(o);
        std::size_t seed = ary.size();
        for (auto& elem : ary)
        {
            seed ^= hash(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
    case object_type_dictionary:
    {
        auto dict        = AS_DICT(o);
        std::size_t seed = dict.size();
        for (auto& it : dict)
        {
            seed ^= hash(it.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash(it.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
    case object_type_struct:
    {
        auto struct_object = AS_STRUCT(o);
        std::size_t seed   = struct_object.fields.size();
        for (auto& field : struct_object.fields)
        {
            seed ^= robin_hood::hash<std::string> {}(field.identifier)
                + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash(field.value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
    case object_type_enum:
    {
        return robin_hood::hash_bytes(
            AS_ENUM(o).variant.c_str(),
            sizeof(char) * AS_ENUM(o).variant.size());
    }
    case object_type_function:
    case object_type_builtin_function:
    case object_type_sequence:
    {
        return robin_hood::hash<void*> {}(nanbox_to_pointer(o.box));
    }
    case object_type_invalid:
    {
        assert(0 && "hash of invalid object");
    }
    }

    assert(0 && "unhandled case in object::hash");
}

}
