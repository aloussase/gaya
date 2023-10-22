#include <cstdlib>
#include <cstring>
#include <filesystem>

#include <fmt/core.h>

#include <eval.hpp>
#include <file_reader.hpp>

#define IGNORE(e) ((void)e)

namespace fs = std::filesystem;

template <typename T>
struct FreeOnExit
{
    FreeOnExit(T* p)
        : ptr(p)
    {
    }

    ~FreeOnExit()
    {
        std::free((void*)ptr);
    }

    T* ptr;
};

struct Error
{
};

struct Success
{
};

template <typename Outcome>
bool expect(const std::string& filename, const char* source) noexcept
{
    return false;
}

template <>
bool expect<Success>(const std::string& filename, const char* source) noexcept
{
    gaya::eval::interpreter interp { nullptr, 0 };
    IGNORE(interp.eval(filename, source));
    return !interp.had_error();
}

template <>
bool expect<Error>(const std::string& filename, const char* source) noexcept
{
    gaya::eval::interpreter interp { nullptr, 0 };
    IGNORE(interp.eval(filename, source));
    return interp.had_error();
}

static const char* slurp(const std::string& filename)
{
    gaya::file_reader reader { filename };
    if (!reader) return nullptr;

    const auto* contents = reader.slurp();
    return contents;
}

static bool test_file(const std::string& filename)
{
    const auto* contents = slurp(filename);
    if (!contents) return false;

    FreeOnExit guard { contents };

    if (std::strstr(contents, "Expect error"))
    {
        return expect<Error>(filename, contents);
    }

    return expect<Success>(filename, contents);
}

auto main() -> int
{
    size_t successes = 0;
    size_t failures  = 0;

    for (auto entry : fs::directory_iterator("tests"))
    {
        auto filename = entry.path().string();
        if (!filename.ends_with(".gaya")) continue;

        if (test_file(filename))
        {
            fmt::println("\x1b[32m{}\x1b[m", filename);
            successes += 1;
        }
        else
        {
            fmt::println("\x1b[31m{}\x1b[m", filename);
            failures += 1;
        }
    }

    fmt::println("Summary: {} successes, {} failures", successes, failures);

    std::exit(failures);
}
