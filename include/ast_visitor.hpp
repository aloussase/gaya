#pragma once

#include <ast.hpp>
#include <object.hpp>

namespace gaya::ast
{

using maybe_object = gaya::eval::object::maybe_object;

class ast_visitor
{
public:
    virtual ~ast_visitor() { }
    virtual maybe_object visit_program(program&)                         = 0;
    virtual maybe_object visit_declaration_stmt(declaration_stmt&)       = 0;
    virtual maybe_object visit_expression_stmt(expression_stmt&)         = 0;
    virtual maybe_object visit_assignment_stmt(assignment_stmt&)         = 0;
    virtual maybe_object visit_while_stmt(while_stmt&)                   = 0;
    virtual maybe_object visit_do_expression(do_expression&)             = 0;
    virtual maybe_object visit_case_expression(case_expression&)         = 0;
    virtual maybe_object visit_call_expression(call_expression&)         = 0;
    virtual maybe_object visit_function_expression(function_expression&) = 0;
    virtual maybe_object visit_let_expression(let_expression&)           = 0;
    virtual maybe_object visit_binary_expression(binary_expression&)     = 0;
    virtual maybe_object visit_unary_expression(unary_expression&)       = 0;
    virtual maybe_object visit_array(array&)                             = 0;
    virtual maybe_object visit_number(number&)                           = 0;
    virtual maybe_object visit_string(string&)                           = 0;
    virtual maybe_object visit_identifier(identifier&)                   = 0;
    virtual maybe_object visit_unit(unit&)                               = 0;
    virtual maybe_object visit_placeholder(placeholder&)                 = 0;
};

}
