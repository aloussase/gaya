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

}
