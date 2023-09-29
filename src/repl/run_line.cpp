#include <eval.hpp>
#include <parser.hpp>

static std::string filename = "<interactive>";
static gaya::parser parser;
static gaya::eval::interpreter interp;

void runLine(const char* line) noexcept
{
    auto ast = parser.parse_expression(filename, line);
    if (ast == nullptr || parser.had_error())
    {
        parser.clear_diagnostics();
        ast = parser.parse_statement(filename, line);
    }

    if (ast == nullptr || parser.had_error())
    {
        parser.report_diagnostics();
        parser.clear_diagnostics();
        return;
    }

    auto o = interp.eval(filename, ast);
    if (!o)
    {
        interp.report_diagnostics();
        interp.clear_diagnostics();
        return;
    }

    printf("= %s\n", gaya::eval::object::to_string(interp, *o).c_str());
}
