#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/core.h>

#include <eval.hpp>
#include <file_reader.hpp>
#include <parser.hpp>
#include <repl.hpp>

static bool run_repl_flag   = false;
static bool show_usage_flag = false;
static bool print_ast_flag  = false;

[[nodiscard]] static bool
print_ast(const char* filename, const char* source) noexcept
{
    auto interp  = gaya::eval::interpreter {};
    auto& parser = interp.get_parser();

    if (auto ast = parser.parse(filename, source);
        ast && parser.diagnostics().empty())
    {
        fmt::println("{}", ast->to_string());
        return true;
    }
    else
    {
        for (const auto& diag : parser.diagnostics())
        {
            fmt::println("{}", diag.to_string());
        }
        return false;
    }
}

[[nodiscard]] static bool run_file(const char* filename, const char* source)
{
    gaya::eval::interpreter interp;
    (void)interp.eval(filename, source);

    if (interp.had_error())
    {
        for (const auto& diagnostic : interp.diagnostics())
        {
            fmt::print("{}", diagnostic.to_string());
        }

        return false;
    }

    return true;
}

[[noreturn]] static void process_file(char* filename)
{
    auto fr = gaya::file_reader { filename };
    if (!fr)
    {
        fmt::println(stderr, "Failed to read file: {}", filename);
        exit(EXIT_FAILURE);
    }

    auto* source = fr.slurp();

    if (print_ast_flag)
    {
        auto ok = print_ast(filename, source);
        free(source);
        exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    else
    {
        auto ok = run_file(filename, source);
        free(source);
        exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
    }
}

[[noreturn]] static void usage()
{
    printf("gaya lang -- version 1.0\n"
           "usage: gaya [--repl] [--help] [--print-ast] [inputfile]\n"
           "arguments:\n"
           "    inputfile    a file to evaluate\n"
           "options:\n"
           "    --repl       run the repl\n"
           "    --help       show this help\n"
           "    --print-ast  do not run the program, print the ast\n");
    exit(EXIT_SUCCESS);
}

auto main(int argc, char** argv) -> int
{
    int i;
    for (i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        if (strcmp(arg, "--repl") == 0)
        {
            run_repl_flag = true;
        }
        else if (strcmp(arg, "--help") == 0)
        {
            show_usage_flag = true;
        }
        else if (strcmp(arg, "--print-ast") == 0)
        {
            print_ast_flag = true;
        }
        else
        {
            break;
        }
    }

    if (show_usage_flag)
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
        process_file(argv[i]);
    }

    if (run_repl_flag)
    {
        gaya::repl::run();
    }

    usage();

    return 0;
}
