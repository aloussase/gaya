#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <numeric>

#include <fmt/core.h>

#include <env.hpp>
#include <eval.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <repl.hpp>

#define CTRL_KEY(c) ((c) & 0x1f)
#define UNUSED(e)   ((void)(e))

namespace gaya::repl
{

static struct termios orig_termios;

static void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

static void enable_raw_mode()
{
    if (!isatty(STDIN_FILENO))
    {
        fmt::println(stderr, "Can only run repl in a terminal");
        exit(EXIT_FAILURE);
    }

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios termios;
    tcgetattr(STDIN_FILENO, &termios);

    tcgetattr(STDIN_FILENO, &termios);
    termios.c_lflag &= ~(ICANON | ECHO);
    termios.c_cc[VMIN]  = 1;
    termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);
}

[[nodiscard]] static std::string highlight(std::string& line)
{

    std::string highlighted;
    std::string word;

    auto is_number = [](std::string& s) {
        auto seen_dot = false;
        for (auto c : s)
        {
            if (!isdigit(c) || (c == '.' && !seen_dot))
            {
                return false;
            }

            if (c == '.')
            {
                seen_dot = true;
            }
        }
        return true;
    };

    auto is_string = [](std::string& s) {
        if (s.empty() || s[0] != '"') return false;

        for (size_t i = 1; i < s.size(); i++)
        {
            char c = s[i];
            if (c == '\\')
            {
                if (!(i < s.size())) return false;
                if (s[i++] == '"') continue;
            }

            if (c == '"') return i == s.size() - 1;
        }

        return false;
    };

    auto is_word_boundary = [](char c) {
        switch (c)
        {
        case ' ':
        case '\n':
        case ')':
        case ',':
        {
            return true;
        }
        default:
        {
            return false;
        }
        }
    };

    auto is_operator = [](std::string& s) {
        return s == "+" || s == "-" || s == "*" || s == "/" || s == "<"
            || s == "<=" || s == ">" || s == ">=" || s == "=" || s == "=="
            || s == "/=";
    };

    for (size_t i = 0; i < line.size(); i++)
    {
        auto c = line[i];
        if (is_word_boundary(c) || i == line.size() - 1)
        {
            if (!is_word_boundary(c)) word += c;
            if (lexer::is_keyword(word))
            {
                highlighted += fmt::format("\x1b[31m{}\x1b[m", word);
            }
            else if (is_number(word))
            {
                highlighted += fmt::format("\x1b[33m{}\x1b[m", word);
            }
            else if (is_string(word))
            {
                highlighted += fmt::format("\x1b[32m{}\x1b[m", word);
            }
            else if (is_operator(word))
            {
                highlighted += fmt::format("\x1b[35m{}\x1b[m", word);
            }
            else
            {
                highlighted += word;
            }

            if (is_word_boundary(c)) highlighted += c;
            word.clear();
        }
        else
        {
            word += c;
        }
    }

    return highlighted;
}

[[nodiscard]] static std::string read_line(std::string prompt) noexcept
{
    fmt::print("{}", prompt);

    char c;
    std::string line;
    int cursor = -1;

    while ((c = fgetc(stdin)) != EOF)
    {
        switch (c)
        {
        case 8:
        case 127: /* Backspace */
        {
            if (cursor >= 0)
            {
                line.erase(cursor, 1);
                cursor -= 1;

                if (static_cast<size_t>(cursor) != line.size() - 1)
                {
                    fmt::print("\x1b[s");
                }

                fmt::print("\x1b[2K\x1b[9999D{}{}", prompt, highlight(line));

                if (static_cast<size_t>(cursor) != line.size() - 1)
                {
                    fmt::print("\x1b[u\x1b[1D");
                }
            }
            break;
        }
        case CTRL_KEY('l'):
        {
            fmt::print("\x1b[2J\x1b[H{}{}", prompt, highlight(line));
            break;
        }
        case '\x1b': /* Arrow keys */
        {
            c = fgetc(stdin);
            if (c == EOF || c != '[') break;

            c = fgetc(stdin);
            if (c == EOF) break;

            switch (c)
            {
            case 'A':
            case 'B': /* TODO: Handle history */
            case 'C':
            {
                if (cursor < 0 || static_cast<size_t>(cursor) < line.size())
                {
                    fmt::print("\x1b[1C");
                    cursor++;
                }
                break;
            }
            case 'D':
            {
                if (cursor >= 0)
                {
                    fmt::print("\x1b[1D");
                    cursor--;
                }
                break;
            }
            default: break;
            }
            break;
        }
        default:
        {
            if (iscntrl(c) && c != '\n') break;
            line += c;
            cursor += 1;
            fmt::print("\x1b[2K\x1b[99999D{}{}", prompt, highlight(line));
        }
        }

        if (c == '\n')
        {
            break;
        }
    }

    return line;
}

void run() noexcept
{
    enable_raw_mode();

    auto interp        = gaya::eval::interpreter {};
    const auto* prompt = "\x1b[35mgaya> \x1b[m";
    auto print_mode    = true;

    fmt::println("Gaya - version 0.1\n"
                 "Enter '.quit' to exit\n"
                 "Enter '.help' to show help\n\n"
                 "Note: Print mode is enabled. All lines will be wrapped in "
                 "io.println.");

    for (;;)
    {
        auto line = read_line(prompt);
        if (line == "\n") continue;

        if (line == ".quit\n")
        {
            fmt::println("Hasta la vista!");
            exit(EXIT_SUCCESS);
        }

        if (line == ".print\n")
        {
            print_mode = !print_mode;
            fmt::println(
                "print mode is {}",
                print_mode ? "enabled" : "disabled");
            continue;
        }

        if (line == ".help\n")
        {
            fmt::println("Commands:\n\n"
                         "  .quit  - Exit the REPL\n"
                         "  .print - Enable print mode\n"
                         "  .help  - Show this help\n\n"
                         "You can enter multiline input by putting "
                         "entering a ':{{' on a separate line.");
            continue;
        }

        if (line == ":{\n")
        {
            std::string multiline;
            while ((line = read_line("|")) != ":}\n")
            {
                multiline += line;
            }
            line = multiline;
        }

        if (!line.starts_with("discard") && line.find("::") == line.npos)
        {
            line = fmt::format(
                "discard {}{}{}",
                print_mode ? "io.println(" : "",
                line,
                print_mode ? ")" : "");
        }

        UNUSED(interp.eval("<interactive>", line.c_str()));

        if (interp.had_error())
        {
            for (const auto& diagnostic : interp.diagnostics())
            {
                fmt::println("{}", diagnostic.to_string());
            }

            interp.clear_diagnostics();
            assert(interp.diagnostics().empty());
        }
    }

    exit(EXIT_SUCCESS);
}

}
