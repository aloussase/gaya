#include <numeric>
#include <sstream>
#include <string>

#include <ast.hpp>
#include <ast_visitor.hpp>

namespace gaya::ast {

template <typename Container, typename Transform>
std::string join(const Container& c, Transform transform)
{
  return std::transform_reduce(
      c.empty() ? c.cbegin() : c.cbegin() + 1,
      c.cend(),
      c.empty() ? std::string() : transform(c[0]),
      [](auto acc, auto x) { return acc + "," + x; },
      transform
  );
}

std::string program::to_string() const noexcept
{
  std::stringstream ss;
  ss << "{\"type\": \"program\","
     << "\"stmts\": [" << join(stmts, [](auto& stmt) { return stmt->to_string(); }) << "]}";
  return ss.str();
}

gaya::eval::object::object_ptr program::accept(ast_visitor& v)
{
  return v.visit_program(*this);
}

std::string declaration_stmt::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "declaration_stmt", "identifier":)" << _identifier->to_string()
     << R"(, "expression": )" << expression->to_string() << "}";
  return ss.str();
}

gaya::eval::object::object_ptr declaration_stmt::accept(ast_visitor& v)
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

gaya::eval::object::object_ptr expression_stmt::accept(ast_visitor& v)
{
  return v.visit_expression_stmt(*this);
}

std::string call_expression::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "call_expression", "args": [)"
     << join(args, [](auto& arg) { return arg->to_string(); }) << R"(], "identifier": ")"
     << identifier->_span.to_string() << "\"}";
  return ss.str();
}

gaya::eval::object::object_ptr call_expression::accept(ast_visitor& v)
{
  return v.visit_call_expression(*this);
}

std::string function_expression::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "function_expression", "params": [)"
     << join(params, [](auto& param) { return param.to_string(); }) << R"(], "body": )"
     << body->to_string() << "}";
  return ss.str();
}

gaya::eval::object::object_ptr function_expression::accept(ast_visitor& v)
{
  return v.visit_function_expression(*this);
}

std::string let_expression::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "let_expression", "identifier": )" << ident->to_string() << R"(, "binding": )"
     << binding->to_string() << R"(, "expr": )" << expr->to_string() << "}";
  return ss.str();
}

gaya::eval::object::object_ptr let_expression::accept(ast_visitor& v)
{
  return v.visit_let_expression(*this);
}

std::string number::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "number", "value":)" << '"' << value << "\"}";
  return ss.str();
}

gaya::eval::object::object_ptr number::accept(ast_visitor& v)
{
  return v.visit_number(*this);
}

std::string string::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "string", "value":)" << value << "}";
  return ss.str();
}

gaya::eval::object::object_ptr string::accept(ast_visitor& v)
{
  return v.visit_string(*this);
}

std::string identifier::to_string() const noexcept
{
  std::stringstream ss;
  ss << R"({"type": "identifier", "value":)" << '"' << value << "\"}";
  return ss.str();
}

gaya::eval::object::object_ptr identifier::accept(ast_visitor& v)
{
  return v.visit_identifier(*this);
}

std::string unit::to_string() const noexcept
{
  return R"({"type": "unit"})";
}

gaya::eval::object::object_ptr unit::accept(ast_visitor& v)
{
  return v.visit_unit(*this);
}

}
