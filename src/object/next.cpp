#include <variant>

#include <nanbox.h>

#include <object.hpp>

namespace gaya::eval::object
{

maybe_object string_sequence_next(span span, string_sequence& seq) noexcept
{
    if (seq.index < seq.string.size())
    {
        std::string string = std::string { seq.string[seq.index++] };
        return create_string(span, string);
    }
    else
    {
        return create_unit(span);
    }
}

maybe_object array_sequence_next(span span, array_sequence& seq) noexcept
{
    if (seq.index < seq.elems.size())
    {
        return seq.elems[seq.index++];
    }
    else
    {
        return create_unit(span);
    }
}

maybe_object number_sequence_next(span span, number_sequence& seq) noexcept
{
    if (seq.i < seq.upto)
    {
        return create_number(span, seq.i++);
    }
    else
    {
        return create_unit(span);
    }
}

maybe_object user_sequence_next(span span, user_defined_sequence& seq) noexcept
{
    return call(seq.next_func, seq.interp, span, {});
}

maybe_object next(sequence& seq) noexcept
{
    switch (seq.type)
    {
    case sequence_type_string:
    {
        return string_sequence_next(
            seq.seq_span,
            std::get<string_sequence>(seq.seq));
    }
    case sequence_type_user:
    {
        return user_sequence_next(
            seq.seq_span,
            std::get<user_defined_sequence>(seq.seq));
    }
    case sequence_type_number:
    {
        return number_sequence_next(
            seq.seq_span,
            std::get<number_sequence>(seq.seq));
    }
    case sequence_type_array:
    {
        return array_sequence_next(
            seq.seq_span,
            std::get<array_sequence>(seq.seq));
    }
    }

    assert(0 && "unhandled case in next");
}

}