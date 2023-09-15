#include <memory>
#include <sstream>

#include <eval.hpp>
#include <sequence.hpp>

namespace gaya::eval::object
{

/* Sequence */

std::string sequence::to_string() noexcept
{
    std::stringstream ss;
    ss << "(";

    auto value = next();
    if (!value) return nullptr;
    if (value->typeof_() != "unit")
    {
        ss << value->to_string();
    }

    for (;;)
    {
        auto value = next();
        if (!value) return nullptr;
        if (value->typeof_() == "unit") break;

        ss << ", " << value->to_string();
    }

    ss << ")";
    return ss.str();
}

bool sequence::is_callable() const noexcept
{
    /* Sequences are not callable */
    return false;
}

std::string sequence::typeof_() const noexcept
{
    return "sequence";
}

bool sequence::is_truthy() const noexcept
{
    return false;
}

bool sequence::is_comparable() const noexcept
{
    /* Sequences are not comparable */
    return false;
}

bool sequence::equals(object_ptr) const noexcept
{
    /* Sequences are not equatable */
    return false;
}

bool sequence::is_sequence() const noexcept
{
    return true;
}

sequence_ptr sequence::to_sequence() noexcept
{
    return shared_from_this();
}

/* String sequences */

object_ptr string_sequence::next() noexcept
{
    if (_index < _contents.size())
    {
        return std::make_shared<string>(
            _span,
            std::string { _contents[_index++] });
    }
    else
    {
        return std::make_shared<unit>(_span);
    }
}

/* Array sequence */

object_ptr array_sequence::next() noexcept
{
    if (_index < _elems.size())
    {
        return _elems[_index++];
    }
    else
    {
        return std::make_shared<unit>(_span);
    }
}

/* Number sequence */

object_ptr number_sequence::next() noexcept
{
    if (_i < _n)
    {
        return std::make_shared<number>(_span, _i++);
    }
    else
    {
        return std::make_shared<unit>(_span);
    }
}

/* Mapper sequence */

object_ptr mapper_sequence::next() noexcept
{
    auto value = _inner->next();
    if (!value) return nullptr;
    if (value->typeof_() == "unit") return value;

    return _mapper->call(_interp, _span, std::vector { value });
}

/* User define sequence */

object_ptr user_defined_sequence::next() noexcept
{
    return _next->call(_interp, _span, {});
}

}
