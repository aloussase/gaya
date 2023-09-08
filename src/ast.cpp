#include <sstream>
#include <string>

#include <ast.hpp>

namespace ast {

std::string program::to_string() const noexcept
{
  std::stringstream ss;
  ss << "{\"type\": \"program\","
     << "\"stmts\": [";
  for (size_t i = 0; i < stmts.size(); i++) {
    ss << stmts[i]->to_string();
    if (i != stmts.size() - 1) {
      ss << ",";
    }
  }
  ss << "]}";
  return ss.str();
}

std::string declaration_stmt::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "declaration_stmt", "identifier":)"
     << _identifier->to_string() << R"(, "expression": )"
     << expression->to_string() << "}";
  return ss.str();
}

std::string expression_stmt::to_string() const noexcept
{
  // TODO
  std::stringstream ss;
  return ss.str();
}

std::string call_expression::to_string() const noexcept
{
  // TODO
  std::stringstream ss;
  return ss.str();
}

std::string function_expression::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "function_expression", "params": [)";
  for (size_t i = 0; i < params.size(); i++) {
    ss << params[i].to_string();
    if (i != params.size() - 1) {
      ss << ",";
    }
  }
  ss << R"(], "body": )" << body->to_string() << "}";
  return ss.str();
}

std::string number::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "number", "value":)" << '"' << value << "\"}";
  return ss.str();
}

std::string string::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "string", "value":)" << '"' << value << "\"}";
  return ss.str();
}

std::string identifier::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "identifier", "value":)" << '"' << value << "\"}";
  return ss.str();
}

std::string unit::to_string() const noexcept
{
  return R"({"type": "unit"})";
}

}
