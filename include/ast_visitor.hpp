#pragma once

#include <ast.hpp>
#include <object.hpp>

namespace gaya::ast
{

class ast_visitor
{
public:
    virtual ~ast_visitor() { }
    virtual eval::object::object visit_program(program&) = 0;

    /* Statements */
    virtual eval::object::object visit_declaration_stmt(declaration_stmt&) = 0;
    virtual eval::object::object visit_expression_stmt(expression_stmt&)   = 0;
    virtual eval::object::object visit_assignment_stmt(assignment_stmt&)   = 0;
    virtual eval::object::object visit_while_stmt(while_stmt&)             = 0;
    virtual eval::object::object visit_for_in_stmt(for_in_stmt&)           = 0;
    virtual eval::object::object visit_include_stmt(include_stmt&)         = 0;
    virtual eval::object::object visit_type_declaration(TypeDeclaration&)  = 0;

    /* Expressions */
    virtual eval::object::object visit_do_expression(do_expression&)       = 0;
    virtual eval::object::object visit_case_expression(case_expression&)   = 0;
    virtual eval::object::object visit_match_expression(match_expression&) = 0;
    virtual eval::object::object visit_call_expression(call_expression&)   = 0;
    virtual eval::object::object visit_function_expression(function_expression&)
        = 0;
    virtual eval::object::object visit_let_expression(let_expression&) = 0;
    virtual eval::object::object visit_binary_expression(binary_expression&)
        = 0;
    virtual eval::object::object visit_lnot_expression(lnot_expression&) = 0;
    virtual eval::object::object visit_not_expression(not_expression&)   = 0;
    virtual eval::object::object visit_perform_expression(perform_expression&)
        = 0;
    virtual eval::object::object visit_array(array&)             = 0;
    virtual eval::object::object visit_dictionary(dictionary&)   = 0;
    virtual eval::object::object visit_number(number&)           = 0;
    virtual eval::object::object visit_string(string&)           = 0;
    virtual eval::object::object visit_identifier(identifier&)   = 0;
    virtual eval::object::object visit_unit(unit&)               = 0;
    virtual eval::object::object visit_placeholder(placeholder&) = 0;
};

}
