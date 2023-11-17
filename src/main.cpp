#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/core.h>

#include <eval.hpp>
#include <file_reader.hpp>
#include <parser.hpp>

#include "repl.hpp"

static bool show_usage_flag = false;
static bool run_repl_flag   = false;

[[noreturn]] static void run_repl(char** argv, int argc)
{
    LineEditor editor;

    gaya::eval::interpreter interp { argv, static_cast<uint32_t>(argc) };
    gaya::parser& parser = interp.get_parser();

    for (;;)
    {
        auto line = editor.read_line();
        if (line.kind() == LineResultKind::Eof
            || line.kind() == LineResultKind::Error)
        {
            fmt::println("Bye-bye!");
            exit(EXIT_SUCCESS);
        }

        if (line.line().empty())
        {
            continue;
        }

        auto source = line.line().c_str();
        auto ast    = parser.parse_expression("<interactive>", source);

        if (parser.had_error())
        {
            parser.clear_diagnostics();
            ast = parser.parse_statement("<interactive>", source);
        }

        if (parser.had_error())
        {
            parser.report_diagnostics();
            parser.clear_diagnostics();
            continue;
        }

        auto object = interp.eval("<interactive>", ast);
        if (interp.had_error())
        {
            interp.report_diagnostics();
            interp.clear_diagnostics();
            continue;
        }

        auto result = object.value();
        auto show   = gaya::eval::object::to_string(interp, result);
        fmt::println("= {}", show);
    }
}

[[nodiscard]] static bool
run_file(const char* filename, const char* source, char** argv, int argc)
{
    gaya::eval::interpreter interp { argv, static_cast<uint32_t>(argc) };
    (void)interp.eval(filename, source);

    if (interp.had_error())
    {
        interp.report_diagnostics();
        return false;
    }

    return true;
}

[[noreturn]] static void process_file(char* filename, char** argv, int argc)
{
    auto fr = gaya::file_reader { filename };
    if (!fr)
    {
        fmt::println(stderr, "Failed to read file: {}", filename);
        exit(EXIT_FAILURE);
    }

    auto* source = fr.slurp();
    auto ok      = run_file(filename, source, argv, argc);
    free(source);
    exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}

[[noreturn]] static void usage()
{
    printf("gaya -- version 1.0\n"
           "usage: gaya [--help] [filename]\n"
           "arguments:\n"
           "    filename     a file to evaluate\n"
           "options:\n"
           "    --help       show this help\n");
    exit(EXIT_SUCCESS);
}

auto main(int argc, char** argv) -> int
{
    int i;
    for (i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        if (strcmp(arg, "--help") == 0)
        {
            show_usage_flag = true;
        }
        else if (strcmp(arg, "--repl") == 0)
        {
            run_repl_flag = true;
        }
        else if (strncmp(arg, "-", 1) == 0 || strncmp(arg, "--", 2) == 0)
        {
            fprintf(stderr, "Invalid option: '%s'\n\n", arg);
            usage();
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

    if (run_repl_flag)
    {
        run_repl(argv, argc);
    }

    auto remaining_args = argc - i;
    if (remaining_args >= 1)
    {
        process_file(argv[i], argv, argc);
    }

    return 0;
}
