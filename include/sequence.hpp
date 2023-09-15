#pragma once

#include <object.hpp>
#include <span.hpp>

namespace gaya::eval
{
class interpreter;
}

namespace gaya::eval::object
{

struct sequence : public object, public std::enable_shared_from_this<sequence>
{
    virtual ~sequence() { }

    std::string to_string() noexcept override;
    bool is_callable() const noexcept override;
    std::string typeof_() const noexcept override;
    bool is_truthy() const noexcept override;
    bool is_comparable() const noexcept override;
    bool equals(object_ptr) const noexcept override;
    bool is_sequence() const noexcept override;
    sequence_ptr to_sequence() noexcept override;

    /**
     * Should return the next element in the sequence, or unit if there are
     * none.
     */
    [[nodiscard]] virtual object_ptr next() noexcept = 0;
};

class string_sequence final : public sequence
{
public:
    string_sequence(span span, const std::string contents)
        : _span { span }
        , _contents { contents }
    {
    }

    object_ptr next() noexcept override;

private:
    span _span;
    std::string _contents = {};
    size_t _index         = 0;
};

class array_sequence final : public sequence
{
public:
    array_sequence(span span, const std::vector<object_ptr> elems)
        : _span { span }
        , _elems { elems }
    {
    }

    object_ptr next() noexcept override;

private:
    span _span;
    std::vector<object_ptr> _elems = {};
    size_t _index                  = 0;
};

/**
 * Acts like Python's range(n).
 */
class number_sequence final : public sequence
{
public:
    number_sequence(span span, double n)
        : _span { span }
        , _n { n }
    {
    }

    object_ptr next() noexcept override;

private:
    span _span;
    double _n;
    double _i = 0;
};

/**
 * TODO: This could be library defined.
 */
class mapper_sequence final : public sequence
{
public:
    mapper_sequence(
        interpreter& interp,
        span span,
        sequence_ptr inner,
        callable_ptr mapper)
        : _interp { interp }
        , _span { span }
        , _inner { inner }
        , _mapper { mapper }
    {
    }

    object_ptr next() noexcept override;

private:
    interpreter& _interp;
    span _span;
    sequence_ptr _inner;
    callable_ptr _mapper;
};

/**
 * A user can create a sequence by providing a next callback. This callback
 * should return successive sequence element, and unit when it is done.
 */
class user_defined_sequence final : public sequence
{
public:
    user_defined_sequence(span span, interpreter& interp, callable_ptr next)
        : _span { span }
        , _interp { interp }
        , _next { next }
    {
    }

    object_ptr next() noexcept override;

private:
    span _span;
    interpreter& _interp;
    callable_ptr _next;
};

}
