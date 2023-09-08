#include <fmt/core.h>

#include <diagnostic.hpp>

namespace diagnostic {

diagnostic::diagnostic(span s, const std::string& m, severity svr)
    : _span { s }
    , _message { m }
    , _severity { svr }
{
}

static diagnostic error(span s, const std::string& m) noexcept
{
  return diagnostic { s, m, severity::error };
}

static diagnostic warning(span s, const std::string& m) noexcept
{
  return diagnostic { s, m, severity::warning };
}

static diagnostic hint(span s, const std::string& m) noexcept
{
  return diagnostic { s, m, severity::hint };
}

std::string diagnostic::to_string() const noexcept
{
  std::string diagnostic_kind;

  switch (_severity) {
  case severity::error: diagnostic_kind = "\x1b[31merror\x1b[m"; break;
  case severity::warning: diagnostic_kind = "\x1b[34mwarning\x1b[m"; break;
  case severity::hint: diagnostic_kind = "\x1b[32mhint\x1b[m"; break;
  }

  return fmt::format(
      "{} at line {}: {}\n\n\tHere --> {}\n", //
      diagnostic_kind,
      _span.lineno(),
      _message,
      _span.to_string()
  );
}

}
