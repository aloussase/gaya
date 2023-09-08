#include <iostream>

#include <parser.hpp>

auto main() -> int
{
  auto program = R"(
hello_world :: { =>
    io.println("Hello, world!")
}

discard io.println(hello_world())
  )";

  parser parser(program);
  auto ast = parser.parse();

  if (parser.diagnostics().empty()) {
    std::cout << ast->to_string() << "\n";
  } else {
    for (const auto& diag : parser.diagnostics()) {
      std::cout << diag.to_string() << "\n";
    }
  }

  return 0;
}
