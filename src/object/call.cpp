#include <fmt/core.h>
#include <nanbox.h>

#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

object call_function(
    function& func,
    interpreter& interp,
    const std::vector<object>& args) noexcept
{
    interp.begin_scope(env { func.closed_over_env });

    for (size_t i = 0; i < args.size(); i++)
    {
        auto arg   = args[i];
        auto param = func.params[i];

        /*
         * NOTE: The parser already checks that the type was previously
         * declared.
         *
         * NOTE: We don't use the param.type directly because it doesn't have
         * the closed over environment.
         */
        if (auto type = interp.get_type(param.type.to_string());
            !type->check(interp, arg))
        {
            interp.interp_error(
                arg.span,
                fmt::format(
                    "Expected an argument of type {}",
                    param.type.to_string()));
            interp.interp_hint(
                arg.span,
                "Make sure that the provided argument satisfies the type's "
                "constraints");
            return invalid;
        }

        if (!interp.match_pattern(arg, param.pattern, &key::param))
        {
            interp.interp_error(
                arg.span,
                "Failed to match pattern with argument in function");
            return invalid;
        }
    }

    auto ret = func.body->accept(interp);
    interp.end_scope();
    return ret;
}

object call_array(
    std::vector<object>& elems,
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    if (!IS_NUMBER(args[0]))
    {
        interp.interp_error(span, "Can only index arrays with numbers");
        return invalid;
    }

    auto i = AS_NUMBER(args[0]);
    if (i < 0 || i >= elems.size())
    {
        interp.interp_error(
            span,
            fmt::format(
                "Invalid index for array of size {}: {}",
                elems.size(),
                i));
        return invalid;
    }

    return elems[i];
}

object call_dict(
    robin_hood::unordered_map<object, object>& dict,
    span span,
    const std::vector<object>& args) noexcept
{
    if (auto it = dict.find(args[0]); it != dict.end())
    {
        return it->second;
    }

    return create_unit(span);
}

object call_string(
    const std::string& string,
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    if (!IS_NUMBER(args[0]))
    {
        interp.interp_error(span, "Can only index strings with numbers");
        return invalid;
    }

    auto i = AS_NUMBER(args[0]);
    if (i < 0 || i >= string.size())
    {
        interp.interp_error(
            span,
            fmt::format(
                "Invalid index for string of size {}: {}",
                string.size(),
                i));
        return invalid;
    }

    return create_string(interp, span, std::string { string[i] });
}

object call(
    object& o,
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    switch (o.type)
    {
    case object_type_string:
    {
        return call_string(AS_STRING(o), interp, span, args);
    }
    case object_type_array:
    {
        return call_array(AS_ARRAY(o), interp, span, args);
    }
    case object_type_dictionary:
    {
        return call_dict(AS_DICT(o), span, args);
    }
    case object_type_function:
    {
        return call_function(AS_FUNCTION(o), interp, args);
    }
    case object_type_builtin_function:
    {
        return AS_BUILTIN_FUNCTION(o).invoke(interp, span, args);
    }
    case object_type_number:
    case object_type_unit:
    case object_type_sequence:
    case object_type_invalid:
    {
        assert(0 && "Should not happen");
    }
    }

    assert(0 && "unhandled case in call");
}

}
