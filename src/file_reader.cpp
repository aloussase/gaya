#include <cstdio>
#include <cstring>

#include <file_reader.hpp>

namespace gaya {

file_reader::file_reader(std::filesystem::path p)
    : _path { p }
{
  if (!std::filesystem::exists(_path)) {
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

  auto* fp = fopen(_path.string().c_str(), "r");
  if (!fp) return {};

  auto filesize  = std::filesystem::file_size(_path);
  auto* contents = (char*)calloc(filesize + 1, sizeof(char));

  char buffer[4096] = { 0 };
  while (fgets(buffer, sizeof(buffer) - 1, fp) != nullptr) {
    // FIXME: This could be faster.
    strcat(contents, buffer);
  }

  return contents;
}

}
