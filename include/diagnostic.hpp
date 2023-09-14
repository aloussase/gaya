#pragma once

#include <span.hpp>

namespace diagnostic
{

enum class severity {
    error,
    warning,
    hint,
};

class diagnostic
{
  public:
    diagnostic(
        span s,
        const std::string&,
        severity svr,
        const std::string& filename = "");

    [[nodiscard]] std::string to_string() const noexcept;

  private:
    span _span;
    std::string _message;
    severity _severity;
    std::string _filename;
};

}
