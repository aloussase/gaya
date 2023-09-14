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
    while (has_next())
    {
        ss << next()->to_string();
        if (has_next())
        {
            ss << ", ";
        }
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
    return has_next();
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

bool string_sequence::has_next() const noexcept
{
    return _index < _contents.size();
}

object_ptr string_sequence::next() noexcept
{
    if (has_next())
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

/* Mapper sequence */

bool mapper_sequence::has_next() const noexcept
{
    return _inner->has_next();
}

object_ptr mapper_sequence::next() noexcept
{
    if (!has_next()) return std::make_shared<unit>(_span);

    auto value = _inner->next();
    if (!value) return nullptr;

    auto mapped = _mapper->call(_interp, _span, std::vector { value });
    return mapped;
}

}
