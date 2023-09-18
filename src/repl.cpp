#include <cstring>
#include <numeric>

#include <fmt/core.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <env.hpp>
#include <eval.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <repl.hpp>

namespace gaya::repl
{

void run() noexcept
{
    eval::interpreter interp {};

    const auto* prompt          = "\x1b[35mgaya> \x1b[m";
    const auto* multiline_begin = ":{";
    const auto* multiline_end   = ":}";

    for (;;)
    {
        char* line = readline(prompt);
        if (!line) break;

        if (strcmp(line, multiline_begin) == 0)
        {
            std::string contents;
            for (;;)
            {
                line = readline("| ");
                if (!line) break;

                if (strcmp(line, multiline_end) == 0)
                {
                    free(line);
                    break;
                }

                contents += line;
                contents += '\n';
                free(line);
            }
            line
                = static_cast<char*>(calloc(contents.size() + 1, sizeof(char)));
            strcpy(line, contents.c_str());
        }

        if (!line) break;

        if (strcmp(line, ".quit") == 0)
        {
            fmt::println("¡Hasta la vista!");
            free(line);
            break;
        }

        auto result = interp.eval("<interactive>", line);

        if (interp.had_error())
        {
            for (auto& diagnostic : interp.diagnostics())
            {
                fmt::print("{}", diagnostic.to_string());
            }

            interp.clear_diagnostics();
            add_history(line);
            free(line);
            continue;
        }

        if (result)
        {
            fmt::print(
                "= {}\n",
                gaya::eval::object::to_string(interp, result.value()));
        }

        add_history(line);
        free(line);
    }

    exit(EXIT_SUCCESS);
}

}
