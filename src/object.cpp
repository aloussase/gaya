#include <cassert>

#include <fmt/core.h>
#include <nanbox.h>
#include <robin_hood.h>

#include <env.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

static int bytes_allocated   = 0;
static int next_gc_threshold = 1024 * 1024;

/*
 * String pool
 *
 * NOTE 1: For this to work, strings need to be treated as immutable values.
 *
 * NOTE 2: As of now, strings never get freed. This should not be a problem for
 * short lived programs like Advent of Code solutions. Implement some sort of
 * weak references if it becomes a problem.
 */
static robin_hood::unordered_map<size_t, object> strings;

static void mark(heap_object* o);

static void mark_array(std::vector<object>& elems)
{
    for (auto& elem : elems)
    {
        if (IS_HEAP_OBJECT(elem))
        {
            mark(AS_HEAP_OBJECT(elem));
        }
    }
}

static void mark_dictionary(robin_hood::unordered_map<object, object>& dict)
{
    for (auto it : dict)
    {
        if (IS_HEAP_OBJECT(it.first))
        {
            mark(AS_HEAP_OBJECT(it.first));
        }

        if (IS_HEAP_OBJECT(it.second))
        {
            mark(AS_HEAP_OBJECT(it.second));
        }
    }
}

static void mark(heap_object* o)
{
    if (o->marked) return;

    o->marked = 1;

    switch (o->type)
    {
    case object_type_array:
    {
        mark_array(o->as_array);
        break;
    }
    case object_type_dictionary:
    {
        mark_dictionary(o->as_dictionary);
        break;
    }
    case object_type_sequence:
    {
        if (auto* user_seq
            = std::get_if<user_defined_sequence>(&o->as_sequence.seq);
            user_seq)
        {
            if (IS_HEAP_OBJECT(user_seq->next_func))
            {
                mark(AS_HEAP_OBJECT(user_seq->next_func));
            }
        }
        else if (auto* array_seq
                 = std::get_if<array_sequence>(&o->as_sequence.seq);
                 array_seq)
        {
            mark_array(array_seq->elems);
        }
        else if (auto* dict_seq
                 = std::get_if<dict_sequence>(&o->as_sequence.seq);
                 dict_seq)
        {
            mark_array(dict_seq->keys);
            mark_array(dict_seq->values);
        }
        break;
    }
    /* TODO: I might need to mark the closed over env. */
    case object_type_function:
    case object_type_builtin_function:
    case object_type_foreign_function:
    case object_type_string:
    case object_type_unit:
    case object_type_number:
    case object_type_invalid:
    {
        break;
    }
    }
}

static void mark_bindings(const env& env)
{
    for (auto& it : env.get_bindings())
    {
        if (IS_HEAP_OBJECT(it.second))
        {
            mark(AS_HEAP_OBJECT(it.second));
        }
    }

    if (auto parent = env.parent(); parent)
    {
        if (parent)
        {
            mark_bindings(*parent);
        }
    }
}

static void mark_scopes(interpreter& interp)
{
    for (const auto& env : interp.scopes())
    {
        mark_bindings(env);
    }
}

static void mark_all(interpreter& interp)
{
    mark_scopes(interp);
}

static void sweep(heap_object* heap_objects)
{
    auto** o = &heap_objects;
    while (*o)
    {
        /* NOTE: We don't collect strings. */
        if ((*o)->marked || (*o)->type == object_type_string)
        {
            (*o)->marked = 0;
            o            = &(*o)->next;
        }
        else
        {
            auto* unreached = *o;
            *o              = unreached->next;
            free(unreached);
        }
    }
}

static void run_gc(interpreter& interp, heap_object* heap_objects)
{
    mark_all(interp);
    sweep(heap_objects);

    next_gc_threshold = bytes_allocated * 2;
}

static object create_object(object_type type, span span)
{
    /* NOTE: Callers should replace the nanbox with whatever is suitable */
    return { type, span, nanbox_empty() };
}

static heap_object* create_heap_object(interpreter& interp)
{
    static heap_object* heap_objects = nullptr;

    auto* o   = static_cast<heap_object*>(malloc(sizeof(heap_object)));
    o->marked = 0;
    o->next   = heap_objects;

    bytes_allocated += sizeof(heap_object);

    if (bytes_allocated >= next_gc_threshold)
    {
        run_gc(interp, heap_objects);
    }

    return o;
}

object create_unit(span span) noexcept
{
    auto o = create_object(object_type_unit, span);
    o.box  = nanbox_null();
    return o;
}

object create_number(span span, double number) noexcept
{
    auto o = create_object(object_type_number, span);
    o.box  = nanbox_from_double(number);
    return o;
}

object create_string(
    interpreter& interp,
    span span,
    const std::string& string) noexcept
{
    return create_string(interp, span, std::string_view { string });
}

[[nodiscard]] object create_string(
    interpreter& interp,
    span span,
    const std::string_view& sv) noexcept
{
    auto hash = robin_hood::hash<std::string_view> {}(sv);

    if (auto it = strings.find(hash); it != strings.end())
    {
        return it->second;
    }

    auto* ptr = create_heap_object(interp);
    new (ptr) heap_object {
        .type      = object_type_string,
        .as_string = std::string { sv },
    };

    auto o = create_object(object_type_string, span);
    o.box  = nanbox_from_pointer(ptr);

    strings.insert({ hash, o });

    return o;
}

object create_array(
    interpreter& interp,
    span span,
    const std::vector<object>& elems) noexcept
{
    auto* ptr = create_heap_object(interp);
    new (ptr) heap_object { .type = object_type_array, .as_array = elems };

    auto o = create_object(object_type_array, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_dictionary(
    interpreter& interp,
    span span,
    const robin_hood::unordered_map<object, object>& dict) noexcept
{
    auto* ptr = create_heap_object(interp);
    new (ptr) heap_object {
        .type          = object_type_dictionary,
        .as_dictionary = dict,
    };

    auto o = create_object(object_type_dictionary, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_builtin_function(
    interpreter& interp,
    const std::string& name,
    size_t arity,
    builtin_function::invoke_t invoke) noexcept
{
    builtin_function function = { arity, name, invoke };
    auto* ptr                 = create_heap_object(interp);
    new (ptr) heap_object {
        .type                = object_type_builtin_function,
        .as_builtin_function = function,
    };

    span span = { 0, nullptr, nullptr };
    auto o    = create_object(object_type_builtin_function, span);
    o.box     = nanbox_from_pointer(ptr);

    return o;
}

object create_foreign_function(
    interpreter& interp,
    std::string libname,
    std::string funcname,
    types::ForeignType return_type,
    std::vector<types::ForeignType> argument_types) noexcept
{
    ForeignFunction function = {
        libname,
        funcname,
        return_type,
        std::move(argument_types),
    };

    auto* ptr = create_heap_object(interp);
    new (ptr) heap_object {
        .type                = object_type_foreign_function,
        .as_foreign_function = function,
    };

    /* TODO: Might want to provide a real span here. */
    span span = { 0, nullptr, nullptr };
    auto o    = create_object(object_type_foreign_function, span);
    o.box     = nanbox_from_pointer(ptr);

    return o;
}

object create_function(
    interpreter& interp,
    span span,
    std::unique_ptr<env> env,
    std::vector<ast::function_param> params,
    std::shared_ptr<ast::expression> body)
{
    function function = { params.size(), std::move(env), params, body };
    auto* ptr         = create_heap_object(interp);
    new (ptr) heap_object {
        .type        = object_type_function,
        .as_function = std::move(function),
    };

    auto o = create_object(object_type_function, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_array_sequence(
    interpreter& interp,
    span span,
    const std::vector<object>& elems) noexcept
{
    auto* ptr = create_heap_object(interp);

    array_sequence array_seq = { elems };
    sequence seq             = { span, sequence_type_array, array_seq };
    new (ptr) heap_object { .type = object_type_sequence, .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_string_sequence(
    interpreter& interp,
    span span,
    const std::string& string) noexcept
{
    auto* ptr = create_heap_object(interp);

    string_sequence string_seq = { string };
    sequence seq               = { span, sequence_type_string, string_seq };
    new (ptr) heap_object { .type = object_type_sequence, .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object
create_number_sequence(interpreter& interp, span span, double number) noexcept
{
    auto* ptr = create_heap_object(interp);

    number_sequence number_seq = { number };
    sequence seq               = { span, sequence_type_number, number_seq };
    new (ptr) heap_object { .type = object_type_sequence, .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object
create_user_sequence(span span, interpreter& interp, object next_func) noexcept
{
    auto* ptr = create_heap_object(interp);

    user_defined_sequence user_seq = { interp, next_func };
    sequence seq                   = { span, sequence_type_user, user_seq };
    new (ptr) heap_object { .type = object_type_sequence, .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

[[nodiscard]] object create_dict_sequence(
    interpreter& interp,
    span span,
    const robin_hood::unordered_map<object, object>& dict) noexcept
{
    auto* ptr = create_heap_object(interp);

    std::vector<object> keys(dict.size(), invalid);
    std::vector<object> values(dict.size(), invalid);

    size_t i = 0;

    for (const auto& [k, v] : dict)
    {
        keys[i]   = k;
        values[i] = v;
        i         = i + 1;
    }

    dict_sequence dict_seq = { keys, values };
    sequence seq           = { span, sequence_type_dict, dict_seq };
    new (ptr) heap_object { .type = object_type_sequence, .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

}
