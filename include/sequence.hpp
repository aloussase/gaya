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
     * Should return whether there are any more elements to consume from this
     * sequence.
     */
    [[nodiscard]] virtual bool has_next() const noexcept = 0;

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

    bool has_next() const noexcept override;
    object_ptr next() noexcept override;

private:
    span _span;
    std::string _contents = {};
    size_t _index         = 0;
};

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

    bool has_next() const noexcept override;
    object_ptr next() noexcept override;

private:
    interpreter& _interp;
    span _span;
    sequence_ptr _inner;
    callable_ptr _mapper;
};

}
