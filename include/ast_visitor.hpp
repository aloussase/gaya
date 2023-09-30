#pragma once

#include <ast.hpp>
#include <object.hpp>

namespace gaya::ast
{

class ast_visitor
{
public:
    using ResultType = gaya::eval::object::object;

    virtual ~ast_visitor() { }
    virtual ResultType visit_program(program&) = 0;

    /* Statements */
    virtual ResultType visit_declaration_stmt(declaration_stmt&)      = 0;
    virtual ResultType visit_expression_stmt(expression_stmt&)        = 0;
    virtual ResultType visit_assignment_stmt(assignment_stmt&)        = 0;
    virtual ResultType visit_while_stmt(while_stmt&)                  = 0;
    virtual ResultType visit_for_in_stmt(for_in_stmt&)                = 0;
    virtual ResultType visit_include_stmt(include_stmt&)              = 0;
    virtual ResultType visit_type_declaration(TypeDeclaration&)       = 0;
    virtual ResultType visit_foreign_declaration(ForeignDeclaration&) = 0;

    /* Expressions */
    virtual ResultType visit_do_expression(do_expression&)             = 0;
    virtual ResultType visit_case_expression(case_expression&)         = 0;
    virtual ResultType visit_match_expression(match_expression&)       = 0;
    virtual ResultType visit_call_expression(call_expression&)         = 0;
    virtual ResultType visit_function_expression(function_expression&) = 0;
    virtual ResultType visit_let_expression(let_expression&)           = 0;
    virtual ResultType visit_binary_expression(binary_expression&)     = 0;
    virtual ResultType visit_lnot_expression(lnot_expression&)         = 0;
    virtual ResultType visit_not_expression(not_expression&)           = 0;
    virtual ResultType visit_perform_expression(perform_expression&)   = 0;
    virtual ResultType visit_array(array&)                             = 0;
    virtual ResultType visit_dictionary(dictionary&)                   = 0;
    virtual ResultType visit_number(number&)                           = 0;
    virtual ResultType visit_string(string&)                           = 0;
    virtual ResultType visit_identifier(identifier&)                   = 0;
    virtual ResultType visit_unit(unit&)                               = 0;
    virtual ResultType visit_placeholder(placeholder&)                 = 0;
};

}
