#include <fmt/core.h>
#include <nanbox.h>

#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

maybe_object call_function(
    function& func,
    interpreter& interp,
    span span,
    std::vector<object> args) noexcept
{
    interp.begin_scope(*func.closed_over_env);
    for (size_t i = 0; i < args.size(); i++)
    {
        auto ident = func.params[i];
        auto arg   = args[i];
        interp.define(ident, arg);
    }
    auto ret = func.body->accept(interp);
    interp.end_scope();
    return ret;
}

maybe_object call_array(
    std::vector<object>& elems,
    interpreter& interp,
    span span,
    std::vector<object> args) noexcept
{
    if (!IS_NUMBER(args[0]))
    {
        interp.interp_error(span, "Can only index arrays with numbers");
        return std::nullopt;
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
        return std::nullopt;
    }

    return elems[i];
}

maybe_object call_string(
    const std::string& string,
    interpreter& interp,
    span span,
    std::vector<object> args) noexcept
{
    if (!IS_NUMBER(args[0]))
    {
        interp.interp_error(span, "Can only index strings with numbers");
        return std::nullopt;
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
        return std::nullopt;
    }

    return create_string(span, std::string { string[i] });
}

std::optional<object> call(
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
        return call_function(AS_FUNCTION(o), interp, span, args);
    }
    case object_type_builtin_function:
    {
        return AS_BUILTIN_FUNCTION(o).invoke(interp, span, args);
    }
    case object_type_number:
    case object_type_unit:
    case object_type_sequence:
    {
        assert(0 && "Should not happen");
    }
    }
}

}
