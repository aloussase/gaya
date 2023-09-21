#include <cassert>
#include <numeric>
#include <sstream>
#include <string>

#include <fmt/core.h>

#include <ast.hpp>
#include <ast_visitor.hpp>
#include <eval.hpp>
#include <object.hpp>

namespace gaya::ast
{

template <typename Container, typename Transform>
std::string join(const Container& c, Transform transform)
{
    return std::transform_reduce(
        c.empty() ? c.cbegin() : c.cbegin() + 1,
        c.cend(),
        c.empty() ? std::string() : transform(c[0]),
        [](auto acc, auto x) { return acc + "," + x; },
        transform);
}

std::string program::to_string() const noexcept
{
    std::stringstream ss;
    ss << "{\"type\": \"program\","
       << "\"stmts\": ["
       << join(stmts, [](auto& stmt) { return stmt->to_string(); }) << "]}";
    return ss.str();
}

object program::accept(ast_visitor& v)
{
    return v.visit_program(*this);
}

std::string declaration_stmt::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "declaration_stmt", "identifier":)" << ident->to_string()
       << R"(, "expression": )" << expr->to_string() << "}";
    return ss.str();
}

object declaration_stmt::accept(ast_visitor& v)
{
    return v.visit_declaration_stmt(*this);
}

std::string expression_stmt::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "expression_stmt",)"
       << R"("expression": )" << expr->to_string() << "}";
    return ss.str();
}

object expression_stmt::accept(ast_visitor& v)
{
    return v.visit_expression_stmt(*this);
}

std::string assignment_stmt::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "assignment_stmt", "identifier":)" << ident->to_string()
       << R"(, "expression": )" << expr->to_string() << "}";
    return ss.str();
}

object assignment_stmt::accept(ast_visitor& v)
{
    return v.visit_assignment_stmt(*this);
}

std::string while_stmt::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "while_stmt", "condition":)" << condition->to_string()
       << R"(, "body": [)"
       << join(body, [](auto& stmt) { return stmt->to_string(); }) << "]}";
    return ss.str();
}

object while_stmt::accept(ast_visitor& v)
{
    return v.visit_while_stmt(*this);
}

std::string include_stmt::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "include_stmt", "file_path":)" << file_path << "}";
    return ss.str();
}

object include_stmt::accept(ast_visitor& v)
{
    return v.visit_include_stmt(*this);
}

/* Expressions */

std::string do_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "do_expression", "body": [)"
       << join(body, [](auto& node) { return node->to_string(); }) << "]}";
    return ss.str();
}

object do_expression::accept(ast_visitor& v)
{
    return v.visit_do_expression(*this);
}

std::string case_branch::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "case_branch", "condition": )" << condition->to_string()
       << R"(, "body": )" << body->to_string() << "}";
    return ss.str();
}

std::string case_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "case_expression", "branches": [)"
       << join(branches, [](auto& branch) { return branch.to_string(); })
       << "]";
    if (otherwise)
    {
        ss << ", \"otherwise\": " << otherwise->to_string();
    }
    ss << "}";
    return ss.str();
}

object case_expression::accept(ast_visitor& v)
{
    return v.visit_case_expression(*this);
}

[[nodiscard]] static std::string
match_pattern_to_string(const match_pattern& pattern) noexcept
{
    std::stringstream ss;
    switch (pattern.kind)
    {
    case match_pattern::kind::wildcard:
    {
        ss << R"("wildcard")";
        break;
    }
    case match_pattern::kind::capture:
    {
        ss << R"({ "type": "capture", "identifier": )"
           << std::get<expression_ptr>(pattern.value)->to_string() << "}";
        break;
    }
    case match_pattern::kind::expr:
    {
        ss << R"({ "type": "expr", "expr": )"
           << std::get<expression_ptr>(pattern.value)->to_string() << "}";
        break;
    }
    case match_pattern::kind::array_pattern:
    {
        auto patterns = std::get<std::vector<match_pattern>>(pattern.value);
        ss << R"({"type": "array_pattern", "patterns": [)";
        for (size_t i = 0; i < patterns.size(); i++)
        {
            ss << match_pattern_to_string(patterns[i]);
            if (i < patterns.size() - 1)
            {
                ss << ", ";
            }
        }
        ss << "]}";
        break;
    }
    }
    return ss.str();
}

std::string match_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "match_expression", "expr": )" << target->to_string()
       << R"(, "branches": [)";
    for (size_t i = 0; i < branches.size(); i++)
    {
        const auto& branch = branches[i];
        ss << R"({ "condition": )"
           << (branch.condition ? branch.condition->to_string() : "null")
           << R"(, "body": )" << branch.body->to_string() << R"(, "pattern": )"
           << match_pattern_to_string(branch.pattern) << "}";
        if (i < branches.size() - 1)
        {
            ss << ", ";
        }
    }
    ss << "]}";
    return ss.str();
}

object match_expression::accept(ast_visitor& v)
{
    return v.visit_match_expression(*this);
}

std::string call_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "call_expression", "args": [)"
       << join(args, [](auto& arg) { return arg->to_string(); })
       << R"(], "identifier": )" << target->to_string() << "}";
    return ss.str();
}

object call_expression::accept(ast_visitor& v)
{
    return v.visit_call_expression(*this);
}

std::string function_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "function_expression", "params": [)"
       << join(params, [](auto& param) { return param.to_string(); })
       << R"(], "body": )" << body->to_string() << "}";
    return ss.str();
}

object function_expression::accept(ast_visitor& v)
{
    return v.visit_function_expression(*this);
}

std::string let_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "let_expression", "bindings": [)"
       << join(
              bindings,
              [](auto& binding) {
                  std::stringstream ss;
                  ss << R"({ "identifier": )" << binding.ident->to_string()
                     << R"(, "binding": )" << binding.value->to_string()
                     << R"(, "value": )" << binding.value->to_string() << "}";
                  return ss.str();
              })
       << "]}";
    return ss.str();
}

object let_expression::accept(ast_visitor& v)
{
    return v.visit_let_expression(*this);
}

object binary_expression::accept(ast_visitor& v)
{
    return v.visit_binary_expression(*this);
}

std::string binary_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "binary_expression", "op": ")" << op.span.to_string()
       << R"(", "lhs": )" << lhs->to_string() << R"(, "rhs": )"
       << rhs->to_string() << "}";
    return ss.str();
}

/* Unary expressions */

std::string lnot_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "lnot_expression", "operand": )" << operand->to_string()
       << "}";
    return ss.str();
}

object lnot_expression::accept(ast_visitor& v)
{
    return v.visit_lnot_expression(*this);
}

std::string not_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "not_expression", "operand": )" << operand->to_string()
       << "}";
    return ss.str();
}

object not_expression::accept(ast_visitor& v)
{
    return v.visit_not_expression(*this);
}

std::string perform_expression::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "perform_expression", "stmt": )" << stmt->to_string()
       << "}";
    return ss.str();
}

object perform_expression::accept(ast_visitor& v)
{
    return v.visit_perform_expression(*this);
}

/* Primary expressions */

std::string array::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "array", "elems": [)"
       << join(elems, [](auto& elem) { return elem->to_string(); }) << "]}";
    return ss.str();
}

object array::accept(ast_visitor& v)
{
    return v.visit_array(*this);
}

std::string dictionary::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "dictionary", "elems": [)";

    for (size_t i = 0; i < keys.size(); i++)
    {
        auto key   = keys[i];
        auto value = values[i];

        ss << R"({"key": )" << key->to_string() << R"(, "value": )"
           << value->to_string() << "}";

        if (i < keys.size() - 1)
        {
            ss << ", ";
        }
    }

    ss << "]}";
    return ss.str();
}

object dictionary::accept(ast_visitor& v)
{
    return v.visit_dictionary(*this);
}

std::string number::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "number", "value":)" << '"' << value << "\"}";
    return ss.str();
}

object number::accept(ast_visitor& v)
{
    return v.visit_number(*this);
}

std::string string::to_string() const noexcept
{
    std::stringstream ss;
    std::string s = "";

    for (size_t i = 0; i < value.size(); i++)
    {
        switch (value[i])
        {
        case '\t': s += "\\t"; break;
        case '\n': s += "\\n"; break;
        case '\b': s += "\\b"; break;
        default: s += value[i];
        }
    }

    ss << R"({"type": "string", "value": ")" << s << "\"}";
    return ss.str();
}

object string::accept(ast_visitor& v)
{
    return v.visit_string(*this);
}

std::string identifier::to_string() const noexcept
{
    std::stringstream ss;
    ss << R"({"type": "identifier", "value":)" << '"' << value << "\"}";
    return ss.str();
}

object identifier::accept(ast_visitor& v)
{
    return v.visit_identifier(*this);
}

std::string unit::to_string() const noexcept
{
    return R"({"type": "unit"})";
}

object unit::accept(ast_visitor& v)
{
    return v.visit_unit(*this);
}

std::string placeholder::to_string() const noexcept
{
    return R"({"type": "placeholder"})";
}

object placeholder::accept(ast_visitor& v)
{
    return v.visit_placeholder(*this);
}

}
