#include <cstring>
#include <numeric>

#include <fmt/core.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <env.hpp>
#include <eval.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <repl.hpp>

namespace gaya::repl
{

std::string stringify_tokens(const std::vector<token>& tokens)
{
    auto begin         = tokens.cbegin();
    auto more_than_one = tokens.size() > 1;
    return std::transform_reduce(
        more_than_one ? begin + 1 : begin,
        tokens.cend(),
        more_than_one ? tokens[0].get_span().to_string() : std::string(),
        [](auto acc, auto c) { return acc + " " + c; },
        [](auto token) { return token.get_span().to_string(); });
}

ast::node_ptr parse_line(const char* line)
{
    auto parser_      = parser { line };
    ast::node_ptr ast = parser_.parse_stmt();

    if (!ast)
    {
        parser_ = parser { line };
        ast     = parser_.parse_expression();
    }

    parser_.merge_diagnostics();

    for (const auto& diag : parser_.diagnostics())
    {
        fmt::print("{}", diag.to_string());
    }

    return ast;
}

void run() noexcept
{
    char* line;
    eval::env repl_env;
    eval::interpreter interp { nullptr };
    auto* prompt = "\x1b[35mgaya> \x1b[m";

    while ((line = readline(prompt)) != nullptr)
    {
        if (strcmp(line, ".quit") == 0)
        {
            fmt::println("¡Hasta la vista!");
            free(line);
            break;
        }

        auto ast = parse_line(line);

        if (!ast)
        {
            add_history(line);
            free(line);
            continue;
        }

        if (auto result = interp.eval(interp.get_env(), std::move(ast)); result)
        {
            fmt::print("= {}\n", result->to_string());
        }

        for (const auto& diag : interp.diagnostics())
        {
            fmt::print("{}", diag.to_string());
        }

        interp.clear_diagnostics();
        add_history(line);
        free(line);
    }

    exit(EXIT_SUCCESS);
}

}
