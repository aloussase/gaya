#include <dlfcn.h>
#include <ffi.h>
#include <gnu/lib-names.h>

#include <fmt/core.h>
#include <nanbox.h>

#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

static ffi_type* foreign_type_to_ffi_type(types::ForeignType ft)
{
    switch (ft)
    {
    case types::ForeignType::c_Void: return &ffi_type_void;
    case types::ForeignType::c_Pointer: return &ffi_type_pointer;
    case types::ForeignType::c_Int: return &ffi_type_sint;
    }
}

object call_foreign_function(
    ForeignFunction& func,
    span span,
    interpreter& interp,
    const std::vector<object>& args)
{
    const char* filename = func.libname == "c" ? LIBC_SO : func.libname.c_str();
    auto* handle         = dlopen(filename, RTLD_LAZY);
    if (!handle)
    {
        interp.interp_error(
            span,
            fmt::format("Failed to load dynamic library {}", func.libname));
        return invalid;
    }

    auto* func_ptr = dlsym(handle, func.funcname.c_str());
    if (!func_ptr)
    {
        interp.interp_error(
            span,
            fmt::format(
                "Failed to load symbol {} from dynamic library {}",
                func.funcname,
                func.libname));
        return invalid;
    }

    auto nargs = func.argument_types.size();

    auto** ffi_args   = (ffi_type**)calloc(nargs, sizeof(ffi_type*));
    auto** ffi_values = (void**)calloc(nargs, sizeof(void*));

    for (size_t i = 0; i < nargs; i++)
    {
        auto arg_type = func.argument_types[i];
        auto arg      = args[i];

        ffi_args[i] = foreign_type_to_ffi_type(arg_type);

        switch (arg.type)
        {
        case object_type_string:
        {
            char* s       = (char*)calloc(1, sizeof(char*));
            ffi_values[i] = &s;
            s             = const_cast<char*>(AS_STRING(arg).c_str());
            break;
        }
        default:
        {
            interp.interp_error(span, "ffi is not supported for other types");
            return invalid;
        }
        }
    }

    auto* return_type = foreign_type_to_ffi_type(func.return_type);

    ffi_cif ffi;
    if (ffi_prep_cif(&ffi, FFI_DEFAULT_ABI, nargs, return_type, ffi_args)
        != FFI_OK)
    {
        interp.interp_error(
            span,
            fmt::format("Failed to prepare FFI call {}", func.funcname));
        return invalid;
    }

    object result = invalid;

    switch (func.return_type)
    {
    case types::ForeignType::c_Int:
    {
        int rc;
        ffi_call(&ffi, (void (*)())(func_ptr), &rc, ffi_values);
        result = create_number(span, rc);
        break;
    }
    case types::ForeignType::c_Pointer:
    {
        /* TODO */
    }
    case types::ForeignType::c_Void:
    {
        ffi_arg rc;
        ffi_call(&ffi, (void (*)())(func_ptr), &rc, ffi_values);
        result = create_unit(span);
        break;
    }
    }

    dlclose(handle);
    free(ffi_args);
    free(ffi_values);

    return result;
}

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

object call_struct(
    StructObject& struct_object,
    interpreter& interp,
    span span,
    const std::vector<object>& args) noexcept
{
    StructObject new_object = struct_object;

    auto n = new_object.fields.size();
    for (size_t i = 0; i < n; i++)
    {
        auto& field = new_object.fields[i];
        auto& value = args[i];

        if (!field.type.check(interp, value))
        {
            interp.interp_error(
                span,
                fmt::format(
                    "Invalid type for field '{}', expected a {}",
                    field.identifier,
                    field.type.to_string()));
            interp.interp_hint(
                span,
                "Check that the type's constraints are satisfied");
            return invalid;
        }

        field.value = value;
    }

    return create_struct_object(
        interp,
        span,
        new_object.name,
        std::move(new_object.fields));
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
    case object_type_foreign_function:
    {
        return call_foreign_function(
            AS_FOREIGN_FUNCTION(o),
            span,
            interp,
            args);
    }
    case object_type_struct:
    {
        return call_struct(AS_STRUCT(o), interp, span, args);
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
