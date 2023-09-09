#include <cstring>

#include <fmt/core.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <env.hpp>
#include <eval.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <repl.hpp>

namespace gaya::repl {

void run() noexcept
{
  char* line;
  eval::env repl_env;
  eval::interpreter interp { nullptr };

  // TODO: Check if there are any left over tokens.

  while ((line = readline("> ")) != nullptr) {
    if (strcmp(line, ".quit") == 0) {
      free(line);
      break;
    }

    auto parser_      = parser { line };
    ast::node_ptr ast = parser_.parse_stmt();

    if (!ast) {
      parser_ = parser { line };
      ast     = parser_.parse_expression();
    }

    if (!ast) {
      for (const auto& diag : parser_.diagnostics()) {
        fmt::print("{}", diag.to_string());
      }
      add_history(line);
      free(line);
      continue;
    }

    auto result = interp.eval(repl_env, std::move(ast));
    repl_env    = interp.get_env();

    if (result) {
      fmt::print("= {}\n", result->to_string());
    }

    for (const auto& diag : interp.diagnostics()) {
      fmt::print("{}", diag.to_string());
    }

    interp.clear_diagnostics();
    add_history(line);
    free(line);
  }
}

}
