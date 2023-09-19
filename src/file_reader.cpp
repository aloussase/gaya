#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include <file_reader.hpp>

namespace gaya
{

file_reader::file_reader(std::filesystem::path p)
    : _path { p }
{
    if (!std::filesystem::exists(_path))
    {
        _valid = false;
    }
}

file_reader::operator bool() const noexcept
{
    return _valid;
}

char* file_reader::slurp() noexcept
{
    if (!_valid) return {};

    auto fd = open(_path.string().c_str(), O_RDONLY);
    if (fd == -1) return {};

    auto filesize  = std::filesystem::file_size(_path);
    auto* contents = (char*)calloc(filesize + 1, sizeof(char));

    if (read(fd, contents, filesize) == -1) return {};

    return contents;
}

}
