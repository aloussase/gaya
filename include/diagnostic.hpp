#pragma once

#include <span.hpp>

namespace diagnostic {

enum class severity {
  error,
  warning,
  hint,
};

class diagnostic {
public:
  diagnostic(span s, const std::string&, severity svr);

  [[nodiscard]] static diagnostic error(span s, const std::string&) noexcept;

  [[nodiscard]] static diagnostic warning(span s, const std::string&) noexcept;

  [[nodiscard]] static diagnostic hint(span s, const std::string&) noexcept;

private:
  span _span;
  std::string _message;
  severity _severity;
};

}
