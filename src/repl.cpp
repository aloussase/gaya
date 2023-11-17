#include <readline/history.h>
#include <readline/readline.h>

#include "repl.hpp"

LineResult::LineResult(LineResultKind kind, std::string line)
    : _kind { kind }
    , _line { std::move(line) }
{
}

LineResult LineResult::eof() noexcept
{
    return { LineResultKind::Eof };
}

LineResult LineResult::error() noexcept
{
    return { LineResultKind::Error };
}

LineResult LineResult::success(std::string line) noexcept
{
    return { LineResultKind::Success, std::move(line) };
}

LineResultKind LineResult::kind() const noexcept
{
    return _kind;
}

const std::string& LineResult::line() const noexcept
{
    return _line;
}

LineEditor::LineEditor(LineEditorOptions options)
    : _options { options }
{
}

LineResult LineEditor::read_line() noexcept
{
    char* line = readline(_options.prompt.c_str());
    if (line == nullptr)
    {
        return LineResult::eof();
    }

    add_history(line);

    std::string line_str { line };
    free(line);

    return LineResult::success(std::move(line_str));
}
