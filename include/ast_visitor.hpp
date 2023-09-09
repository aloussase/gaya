#pragma once

#include <ast.hpp>
#include <object.hpp>

namespace ast {

class ast_visitor {
public:
  virtual ~ast_visitor() { }
  virtual object::object_ptr visit_program(program&)                         = 0;
  virtual object::object_ptr visit_declaration_stmt(declaration_stmt&)       = 0;
  virtual object::object_ptr visit_expression_stmt(expression_stmt&)         = 0;
  virtual object::object_ptr visit_call_expression(call_expression&)         = 0;
  virtual object::object_ptr visit_function_expression(function_expression&) = 0;
  virtual object::object_ptr visit_let_expression(let_expression&)           = 0;
  virtual object::object_ptr visit_number(number&)                           = 0;
  virtual object::object_ptr visit_string(string&)                           = 0;
  virtual object::object_ptr visit_identifier(identifier&)                   = 0;
  virtual object::object_ptr visit_unit(unit&)                               = 0;
};

}
