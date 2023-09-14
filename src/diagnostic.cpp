#include <numeric>
#include <sstream>

#include <fmt/core.h>

#include <diagnostic.hpp>

namespace diagnostic
{

diagnostic::diagnostic(
    span s,
    const std::string& m,
    severity svr,
    const std::string& filename)
    : _span { s }
    , _message { m }
    , _severity { svr }
    , _filename { filename }
{
}

std::string diagnostic::to_string() const noexcept
{
    std::string diagnostic_kind = ([&] {
        switch (_severity)
        {
        case severity::error: return "\x1b[1m\x1b[31merror\x1b[m";
        case severity::warning: return "\x1b[1m\x1b[33mwarning\x1b[m";
        case severity::hint: return "\x1b[1m\x1b[34mhint\x1b[m";
        }
    })();

    std::stringstream ss;

    ss << fmt::format(
        "{} at {}{}: {}\n\n",
        diagnostic_kind,
        _filename != "" ? fmt::format("{}:", _filename) : "",
        _span.lineno(),
        _message);

    if (_severity == severity::error)
    {
        const char* newline = strchr(_span.start(), '\n');
        auto line           = std::accumulate(
            _span.start(),
            newline ? newline : strchr(_span.start(), '\0'),
            std::string(),
            [](auto acc, char c) { return acc + c; });
        ss << fmt::format("    Here --> \x1b[4:3m{}\x1b[m\n\n", line);
    }

    return ss.str();
}

}
