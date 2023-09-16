#include <cassert>

#include <nanbox.h>

#include <eval.hpp>
#include <object.hpp>

namespace gaya::eval::object
{

static object create_object(object_type type, span span)
{
    /* NOTE: Callers should replace the nanbox with whatever is suitable */
    return { type, span, nanbox_empty() };
}

static heap_object* create_heap_object()
{
    static heap_object* heap_objects = nullptr;
    auto* o = static_cast<heap_object*>(malloc(sizeof(heap_object)));
    assert(o);
    o->next = heap_objects;
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

object create_string(span span, const std::string& string) noexcept
{
    auto* ptr = create_heap_object();
    new (ptr) heap_object { .as_string = string };

    auto o = create_object(object_type_string, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_array(span span, const std::vector<object>& elems) noexcept
{
    auto* ptr = create_heap_object();
    new (ptr) heap_object { .as_array = elems };

    auto o = create_object(object_type_array, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_builtin_function(
    const std::string& name,
    size_t arity,
    builtin_function::invoke_t invoke) noexcept
{
    builtin_function function = { arity, name, invoke };
    auto* ptr                 = create_heap_object();
    new (ptr) heap_object { .as_builtin_function = function };

    span span = { 0, nullptr, nullptr };
    auto o    = create_object(object_type_builtin_function, span);
    o.box     = nanbox_from_pointer(ptr);

    return o;
}

object create_function(
    span span,
    std::unique_ptr<env> env,
    std::vector<key> params,
    std::shared_ptr<ast::expression> body)
{
    function function = { params.size(), std::move(env), params, body };
    auto* ptr         = create_heap_object();
    new (ptr) heap_object { .as_function = std::move(function) };

    auto o = create_object(object_type_function, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object
create_array_sequence(span span, const std::vector<object>& elems) noexcept
{
    auto* ptr = create_heap_object();

    array_sequence array_seq = { elems };
    sequence seq             = { span, sequence_type_array, array_seq };
    new (ptr) heap_object { .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_string_sequence(span span, const std::string& string) noexcept
{
    auto* ptr = create_heap_object();

    string_sequence string_seq = { string };
    sequence seq               = { span, sequence_type_string, string_seq };
    new (ptr) heap_object { .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object create_number_sequence(span span, double number) noexcept
{
    auto* ptr = create_heap_object();

    number_sequence number_seq = { number };
    sequence seq               = { span, sequence_type_number, number_seq };
    new (ptr) heap_object { .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

object
create_user_sequence(span span, interpreter& interp, object next_func) noexcept
{
    auto* ptr = create_heap_object();

    user_defined_sequence user_seq = { interp, next_func };
    sequence seq                   = { span, sequence_type_user, user_seq };
    new (ptr) heap_object { .as_sequence = seq };

    auto o = create_object(object_type_sequence, span);
    o.box  = nanbox_from_pointer(ptr);

    return o;
}

}
