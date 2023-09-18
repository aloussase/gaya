#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include <nanbox.h>
#include <robin_hood.h>

#include <span.hpp>

#define IS_NUMBER(o)      ((o).type == gaya::eval::object::object_type_number)
#define IS_STRING(o)      ((o).type == gaya::eval::object::object_type_string)
#define IS_HEAP_OBJECT(o) (nanbox_is_pointer((o).box))

#define AS_NUMBER(o)           nanbox_to_double((o).box)
#define AS_HEAP_OBJECT(o)      static_cast<heap_object*>(nanbox_to_pointer((o).box))
#define AS_STRING(o)           AS_HEAP_OBJECT(o)->as_string
#define AS_ARRAY(o)            AS_HEAP_OBJECT(o)->as_array
#define AS_DICT(o)             AS_HEAP_OBJECT(o)->as_dictionary
#define AS_FUNCTION(o)         AS_HEAP_OBJECT(o)->as_function
#define AS_BUILTIN_FUNCTION(o) AS_HEAP_OBJECT(o)->as_builtin_function
#define AS_SEQUENCE(o)         AS_HEAP_OBJECT(o)->as_sequence

namespace gaya::ast
{
struct expression;
}

namespace gaya::eval
{
class interpreter;
class env;
struct key;
}

namespace gaya::eval::object
{

enum object_type {
    object_type_invalid,
    object_type_number,
    object_type_unit,
    object_type_string,
    object_type_array,
    object_type_dictionary,
    object_type_function,
    object_type_builtin_function,
    object_type_sequence,
};

struct object
{
    object_type type;
    class span span;
    nanbox_t box;
};

/*
 * Special object to signal an error return
 * in the interpreter.
 */
static object invalid = object {
    .type = object_type_invalid,
    .span = { 1, nullptr, nullptr },
    .box  = { nanbox_empty() },
};

[[nodiscard]] static inline bool is_valid(object& o) noexcept
{
    return o.type != object_type_invalid;
}

/* Heap objects */

/* Functions */

struct function final
{
    size_t arity;
    std::shared_ptr<env> closed_over_env;
    std::vector<key> params;
    std::shared_ptr<ast::expression> body;
};

struct builtin_function
{
    using invoke_t
        = std::function<object(interpreter&, span, const std::vector<object>&)>;

    size_t arity;
    std::string name;
    invoke_t invoke;
};

/* Sequences */

struct string_sequence
{
    std::string string;
    size_t index = 0;
};

struct array_sequence final
{
    std::vector<object> elems;
    size_t index = 0;
};

struct number_sequence final
{
    double upto;
    double i = 0;
};

struct user_defined_sequence final
{
    interpreter& interp;
    object next_func;
};

enum sequence_type {
    sequence_type_string,
    sequence_type_array,
    sequence_type_number,
    sequence_type_user,
};

struct sequence
{
    span seq_span;
    sequence_type type;
    std::variant<
        string_sequence,
        array_sequence,
        number_sequence,
        user_defined_sequence>
        seq;
};

[[nodiscard]] bool equals(const object&, const object&) noexcept;
[[nodiscard]] size_t hash(const object&) noexcept;

}

namespace std
{

template <>
struct hash<gaya::eval::object::object>
{
    size_t operator()(const gaya::eval::object::object& o) const
    {
        return gaya::eval::object::hash(o);
    }
};

template <>
struct equal_to<gaya::eval::object::object>
{
    bool operator()(
        const gaya::eval::object::object& lhs,
        const gaya::eval::object::object& rhs) const
    {
        return gaya::eval::object::equals(lhs, rhs);
    }
};

}

namespace gaya::eval::object
{

struct heap_object
{
    object_type type;
    union {
        std::vector<object> as_array;
        robin_hood::unordered_map<object, object> as_dictionary;
        std::string as_string;
        function as_function;
        builtin_function as_builtin_function;
        sequence as_sequence;
    };
    unsigned char marked     = 0;
    struct heap_object* next = nullptr;
};

/* Api */

/* Constructors */

/**
 * Create a unit object.
 */
[[nodiscard]] object create_unit(span) noexcept;

/**
 * Create a number.
 */
[[nodiscard]] object create_number(span, double) noexcept;

/**
 * Create a string object.
 */
[[nodiscard]] object
create_string(interpreter&, span, const std::string&) noexcept;

/**
 * Create an array object.
 */
[[nodiscard]] object
create_array(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Create a dictionary object.
 */
[[nodiscard]] object create_dictionary(
    interpreter&,
    span,
    const robin_hood::unordered_map<object, object>&) noexcept;

/**
 * Create a function object.
 */
[[nodiscard]] object create_function(
    interpreter&,
    span,
    std::unique_ptr<env>,
    std::vector<key>,
    std::shared_ptr<ast::expression>);

/**
 * Create a builtin function object.
 */
[[nodiscard]] object create_builtin_function(
    interpreter&,
    const std::string&,
    size_t,
    builtin_function::invoke_t) noexcept;

/**
 * Create an array sequence object.
 */
[[nodiscard]] object
create_array_sequence(interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Create a string sequence object.
 */
[[nodiscard]] object
create_string_sequence(interpreter&, span, const std::string&) noexcept;

/**
 * Create a number sequence object.
 */
[[nodiscard]] object
create_number_sequence(interpreter&, span, double) noexcept;

/**
 * Create a user defined sequence object.
 */
[[nodiscard]] object create_user_sequence(span, interpreter&, object) noexcept;

/* Operations */

/**
 * Convert the provided object to a string representation.
 */
[[nodiscard]] std::string to_string(interpreter&, object) noexcept;

/**
 * Return a string representing the type of the object.
 */
[[nodiscard]] std::string typeof_(const object&) noexcept;

/**
 * Return whether the given object is truthy.
 */
[[nodiscard]] bool is_truthy(const object&) noexcept;

/**
 * Return whether the given object can be compared.
 */
[[nodiscard]] bool is_comparable(const object&) noexcept;

/**
 * Compare two objects.
 */
[[nodiscard]] bool cmp(const object&, const object&, int*) noexcept;

/**
 * Return whether two objects are equal to each other.
 */
[[nodiscard]] bool equals(const object&, const object&) noexcept;

/**
 * Return a hash of the given object.
 */
[[nodiscard]] size_t hash(const object&) noexcept;

/**
 * Return whether the given object is callable or not.
 */
[[nodiscard]] bool is_callable(const object&) noexcept;

/**
 * For callables, return the arity of the callable.
 */
[[nodiscard]] size_t arity(const object&) noexcept;

/**
 * For callables, invoke the callable.
 */
[[nodiscard]] object
call(object&, interpreter&, span, const std::vector<object>&) noexcept;

/**
 * Return whether a given object participates in the sequence protocol.
 */
[[nodiscard]] bool is_sequence(const object&) noexcept;

/**
 * Return the corresponding sequence.
 */
[[nodiscard]] object to_sequence(interpreter&, object&) noexcept;

/**
 * Should return the next element in the sequence, or an empty optional if there
 * are none.
 */
[[nodiscard]] object next(interpreter&, sequence&) noexcept;

}
