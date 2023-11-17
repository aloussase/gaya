#pragma once

#include <string>

struct LineEditorOptions final
{
    std::string prompt;
    std::string multiline_prompt;
};

static LineEditorOptions DEFAULT_LINE_EDITOR_OPTIONS = {
    .prompt           = "> ",
    .multiline_prompt = "| ",
};

enum class LineResultKind {
    Success,
    Error,
    Eof,
};

class LineResult
{
public:
    LineResult(LineResultKind, std::string = "");

    static LineResult eof() noexcept;

    static LineResult error() noexcept;

    static LineResult success(std::string line) noexcept;

    [[nodiscard]] LineResultKind kind() const noexcept;

    [[nodiscard]] const std::string& line() const noexcept;

private:
    LineResultKind _kind;
    std::string _line;
};

class LineEditor final
{
public:
    LineEditor(LineEditorOptions = DEFAULT_LINE_EDITOR_OPTIONS);

    [[nodiscard]] LineResult read_line() noexcept;

private:
    LineEditorOptions _options;
};
