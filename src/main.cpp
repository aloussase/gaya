#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/core.h>

#include <eval.hpp>
#include <file_reader.hpp>
#include <parser.hpp>
#include <repl.hpp>

static bool run_repl   = false;
static bool show_usage = false;
static bool print_ast  = false;

static void eval_file(char* filename)
{
    auto fr = gaya::file_reader { filename };
    if (!fr)
    {
        fmt::println(stderr, "Failed to read file: {}", filename);
        exit(EXIT_FAILURE);
    }

    auto* contents = fr.slurp();
    if (print_ast)
    {
        gaya::parser parser { contents };
        if (auto ast = parser.parse(); ast)
        {
            fmt::println("{}", ast->to_string());
        }
        else
        {
            for (const auto& diag : parser.diagnostics())
            {
                fmt::println("{}", diag.to_string());
            }
            free(contents);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        auto interp = gaya::eval::interpreter { contents };
        if (auto result = interp.eval(); !result)
        {
            for (const auto& diag : interp.diagnostics())
            {
                fmt::print("{}", diag.to_string());
            }
            free(contents);
            exit(EXIT_FAILURE);
        }
    }

    free(contents);
    exit(EXIT_SUCCESS);
}

static void usage()
{
    printf("gaya lang -- version 1.0\n"
           "usage: gaya [--repl] [--help] [--print-ast] [inputfile]\n"
           "arguments:\n"
           "    inputfile    a file to evaluate\n"
           "options:\n"
           "    --repl       run the repl\n"
           "    --help       show this help\n"
           "    --print-ast  do not run the program, print the ast\n");
    exit(EXIT_FAILURE);
}

auto main(int argc, char** argv) -> int
{
    int i;
    for (i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        if (strcmp(arg, "--repl") == 0)
        {
            run_repl = true;
        }
        else if (strcmp(arg, "--help") == 0)
        {
            show_usage = true;
        }
        else if (strcmp(arg, "--print-ast") == 0)
        {
            print_ast = true;
        }
        else
        {
            break;
        }
    }

    if (show_usage)
    {
        usage();
    }

    auto remaining_args = argc - i;
    if (remaining_args > 1)
    {
        usage();
    }

    if (remaining_args == 1)
    {
        auto inputfile = argv[i];
        eval_file(inputfile);
    }

    if (run_repl)
    {
        gaya::repl::run();
    }

    usage();

    return 0;
}
