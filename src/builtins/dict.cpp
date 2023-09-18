#include <builtins/dict.hpp>
#include <eval.hpp>

namespace gaya::eval::object::builtin::dict
{

gaya::eval::object::object
length(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    using namespace gaya::eval::object;

    auto& d = args[0];

    if (d.type != object_type_dictionary)
    {
        interp.interp_error(span, "Expected first argument to be a dictionary");
        return invalid;
    }

    return create_number(span, AS_DICT(d).size());
}

gaya::eval::object::object
insert(interpreter& interp, span span, const std::vector<object>& args) noexcept
{
    auto& d = args[0];
    auto& k = args[1];
    auto& v = args[2];

    if (d.type != object_type_dictionary)
    {
        interp.interp_error(span, "Expected first argument to be a dictionary");
        return invalid;
    }

    AS_DICT(d).insert({ k, v });

    return d;
}

}
