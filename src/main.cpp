#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fmt/core.h>

#include <eval.hpp>
#include <file_reader.hpp>
#include <parser.hpp>

static bool show_usage_flag = false;

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

    auto remaining_args = argc - i;
    if (remaining_args >= 1)
    {
        process_file(argv[i], argv, argc);
    }

    return 0;
}
