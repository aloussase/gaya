#pragma once

#include <cstddef>
#include <string>

class span
{
  public:
    constexpr span(size_t lineno, const char* start, const char* end)
        : _lineno { lineno }
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

    std::string to_string() const noexcept;

  private:
    size_t _lineno;
    const char* _start;
    const char* _end;
};
