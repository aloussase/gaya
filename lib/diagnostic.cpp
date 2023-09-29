#include <numeric>
#include <sstream>
#include <vector>

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

static std::vector<std::string> get_lines(const char* source) noexcept
{
    std::vector<std::string> lines;
    std::string current_line;

    for (size_t i = 0; source[i] != '\0'; i++)
    {
        if (source[i] == '\n')
        {
            lines.push_back(current_line);
            current_line.clear();
        }
        else
        {
            current_line.push_back(source[i]);
        }
    }

    lines.push_back(current_line);

    return lines;
}

enum class StyleOption {
    Dim,
    Undercurl,
    None,
};

static std::string linum_with_line(
    size_t linum,
    const std::string& line,
    StyleOption option          = StyleOption::None,
    bool should_highlight_linum = false) noexcept
{
    if (line.empty()) return "";

    std::stringstream ss;

    if (should_highlight_linum)
    {
        ss << fmt::format("    \x1b[31m{} |\x1b[m ", linum);
    }
    else
    {
        ss << fmt::format("    \x1b[38:5:145m{} |\x1b[m ", linum);
    }

    switch (option)
    {
    case StyleOption::Dim:
    {
        ss << fmt::format("\x1b[38:5:145m{}\x1b[m\n", line);
        break;
    }
    case StyleOption::Undercurl:
    {
        ss << fmt::format("\x1b[4:3m{}\x1b[m\n", line);
        break;
    }
    case StyleOption::None:
    {
        ss << fmt::format("{}\n", line);
        break;
    }
    }

    return ss.str();
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

    ss << fmt::format("{}", diagnostic_kind);

    if (_severity == severity::error)
    {
        ss << fmt::format(
            " at {}{}",
            _filename != "" ? fmt::format("{}:", _filename) : "",
            _span.lineno());
    }

    ss << fmt::format(": {}\n\n", _message);

    if (_severity == severity::error)
    {
        auto lines = get_lines(_span.source());

        int linum           = _span.lineno() - 1;
        auto context_before = linum - 1 > 0 ? lines[linum - 1] : "";
        auto line           = lines[linum];
        auto context_after  = linum + 1 < lines.size() ? lines[linum + 1] : "";

        ss << linum_with_line(linum, context_before, StyleOption::Dim);
        ss << linum_with_line(linum + 1, line, StyleOption::Undercurl, true);
        ss << linum_with_line(linum + 2, context_after, StyleOption::Dim);
    }

    ss << "\n";

    return ss.str();
}

}
