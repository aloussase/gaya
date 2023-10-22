#pragma once

#include <cstddef>
#include <string>

class span
{
public:
    static span invalid;

    constexpr span(
        const char* source,
        size_t lineno,
        const char* start,
        const char* end)
        : _source { source }
        , _lineno { lineno }
        , _start { start }
        , _end { end }
    {
    }

    [[nodiscard]] constexpr size_t lineno() const noexcept
    {
        return _lineno;
    }

    [[nodiscard]] constexpr const char* start() const noexcept
    {
        return _start;
    }

    [[nodiscard]] constexpr const char* end() const noexcept
    {
        return _end;
    }

    [[nodiscard]] constexpr const char* source() const noexcept
    {
        return _source;
    }

    std::string to_string() const noexcept;

private:
    const char* _source;
    size_t _lineno;
    const char* _start;
    const char* _end;
};
