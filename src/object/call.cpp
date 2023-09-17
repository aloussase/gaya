#include <fmt/core.h>
#include <nanbox.h>

#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

object call_function(
    function& func,
    interpreter& interp,
    std::vector<object> args) noexcept
{
    interp.begin_scope(env { func.closed_over_env });
    for (size_t i = 0; i < args.size(); i++)
    {
        interp.define(func.params[i], args[i]);
    }
    auto ret = func.body->accept(interp);
    interp.end_scope();
    return ret;
}

object call_array(
    std::vector<object>& elems,
    interpreter& interp,
    span span,
    std::vector<object> args) noexcept
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

object call_string(
    const std::string& string,
    interpreter& interp,
    span span,
    std::vector<object> args) noexcept
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
    object o,
    interpreter& interp,
    span span,
    std::vector<object> args) noexcept
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
