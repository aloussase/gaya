#pragma once

#include <filesystem>

namespace gaya
{

class file_reader
{
public:
    file_reader(std::filesystem::path);

    /// Dynamically allocated memory should be relased.
    [[nodiscard]] char* slurp() noexcept;

    [[nodiscard]] operator bool() const noexcept;

private:
    std::filesystem::path _path;
    bool _valid = true;
};

}
