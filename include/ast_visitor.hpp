#pragma once

#include <ast.hpp>
#include <object.hpp>

namespace gaya::ast
{

namespace o = gaya::eval::object;

class ast_visitor
{
  public:
    virtual ~ast_visitor() { }
    virtual o::object_ptr visit_program(program&)                         = 0;
    virtual o::object_ptr visit_declaration_stmt(declaration_stmt&)       = 0;
    virtual o::object_ptr visit_expression_stmt(expression_stmt&)         = 0;
    virtual o::object_ptr visit_assignment_stmt(assignment_stmt&)         = 0;
    virtual o::object_ptr visit_while_stmt(WhileStmt&)                    = 0;
    virtual o::object_ptr visit_do_expression(do_expression&)             = 0;
    virtual o::object_ptr visit_case_expression(case_expression&)         = 0;
    virtual o::object_ptr visit_call_expression(call_expression&)         = 0;
    virtual o::object_ptr visit_function_expression(function_expression&) = 0;
    virtual o::object_ptr visit_let_expression(let_expression&)           = 0;
    virtual o::object_ptr visit_binary_expression(binary_expression&)     = 0;
    virtual o::object_ptr visit_unary_expression(unary_expression&)       = 0;
    virtual o::object_ptr visit_number(number&)                           = 0;
    virtual o::object_ptr visit_string(string&)                           = 0;
    virtual o::object_ptr visit_identifier(identifier&)                   = 0;
    virtual o::object_ptr visit_unit(unit&)                               = 0;
};

}
