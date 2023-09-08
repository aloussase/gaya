#include <iostream>

#include <parser.hpp>

auto main() -> int
{
  auto program = R"(
hello_world :: {
    io.println("Hello, world!")
    io.println(12)
}

hello_world()
  )";

  parser parser(program);
  auto ast = parser.parse();

  // TODO: Print parser errors.

  std::cout << ast->to_string() << "\n";

  return 0;
}
