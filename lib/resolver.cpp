#include <variant>

#include <fmt/core.h>

#include <object.hpp>
#include <resolver.hpp>

namespace gaya
{

Resolver::Resolver(std::vector<scope> scopes)
    : _scopes { std::move(scopes) }
{
}

void Resolver::begin_scope() noexcept
{
    _scopes.push_back({});
}

void Resolver::end_scope() noexcept
{
    assert(_scopes.size() > 0);
    _scopes.pop_back();
}

std::vector<diagnostic::diagnostic> Resolver::diagnostics() const noexcept
{
    return _diagnostics;
}

void Resolver::assign_scope(ast::identifier& ident) noexcept
{
    assert(_scopes.size() > 0);

    if (ident.did_assign_scope) return;

    for (int i = _scopes.size() - 1; i >= 0; i--)
    {
        if (_scopes[i].contains(ident.key))
        {
            ident.depth            = _scopes.size() - 1 - i;
            ident.did_assign_scope = true;
            return;
        }
    }

    _diagnostics.emplace_back(
        ident._span,
        fmt::format("Undefined identifier '{}'", ident.value),
        diagnostic::severity::error);
}

void Resolver::resolve(ast::node_ptr ast)
{
    ast->accept(*this);
}

eval::object::object Resolver::visit_program(ast::program& program)
{
    /* We start with the global parser scope. */
    assert(_scopes.size() == 1);

    for (auto& stmt : program.stmts)
    {
        stmt->accept(*this);
    }

    return eval::object::invalid;
}

eval::object::object
Resolver::visit_declaration_stmt(ast::declaration_stmt& declaration)
{
    declaration.expr->accept(*this);
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_expression_stmt(ast::expression_stmt& expression_stmt)
{
    expression_stmt.expr->accept(*this);
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_assignment_stmt(ast::assignment_stmt& assignment)
{
    /* We don't resolve the target because assignment cannot be performed on
     * top-level bindings. */
    assignment.expression->accept(*this);
    return eval::object::invalid;
}

eval::object::object Resolver::visit_while_stmt(ast::while_stmt& while_stmt)
{
    begin_scope();

    if (while_stmt.init)
    {
        while_stmt.init->value->accept(*this);
    }

    while_stmt.condition->accept(*this);

    if (while_stmt.continuation)
    {
        while_stmt.continuation->accept(*this);
    }

    for (auto& stmt : while_stmt.body)
    {
        stmt->accept(*this);
    }

    end_scope();
    return eval::object::invalid;
}

eval::object::object Resolver::visit_for_in_stmt(ast::for_in_stmt& for_stmt)
{
    begin_scope();

    for_stmt.sequence->accept(*this);

    for (auto& stmt : for_stmt.body)
    {
        stmt->accept(*this);
    }

    end_scope();
    return eval::object::invalid;
}

eval::object::object Resolver::visit_include_stmt(ast::include_stmt&)
{
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_type_declaration(ast::TypeDeclaration& type_decl)
{
    if (type_decl.constraint)
    {
        begin_scope();
        type_decl.constraint->accept(*this);
        end_scope();
    }
    return eval::object::invalid;
}

eval::object::object Resolver::visit_struct_declaration(ast::StructDeclaration&)
{
    return eval::object::invalid;
}

eval::object::object Resolver::visit_do_expression(ast::do_expression& do_)
{
    begin_scope();

    for (auto& node : do_.body)
    {
        node->accept(*this);
    }

    end_scope();
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_case_expression(ast::case_expression& case_expression)
{
    for (auto& branch : case_expression.branches)
    {
        branch.condition->accept(*this);
        branch.body->accept(*this);
    }

    if (case_expression.otherwise)
    {
        case_expression.otherwise->accept(*this);
    }

    return eval::object::invalid;
}

eval::object::object
Resolver::visit_match_expression(ast::match_expression& match_expression)
{
    match_expression.target->accept(*this);

    for (auto& branch : match_expression.branches)
    {
        begin_scope();

        if (branch.condition) branch.condition->accept(*this);
        branch.body->accept(*this);

        end_scope();
    }

    if (match_expression.otherwise)
    {
        match_expression.otherwise->accept(*this);
    }

    return eval::object::invalid;
}

eval::object::object
Resolver::visit_lnot_expression(ast::lnot_expression& lnot_expression)
{
    lnot_expression.operand->accept(*this);
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_not_expression(ast::not_expression& not_expression)
{
    not_expression.operand->accept(*this);
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_perform_expression(ast::perform_expression& perform)
{
    perform.stmt->accept(*this);
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_binary_expression(ast::binary_expression& binop)
{
    binop.lhs->accept(*this);

    if (binop.op.type == token_type::pipe)
    {
        begin_scope();
    }

    binop.rhs->accept(*this);

    if (binop.op.type == token_type::pipe)
    {
        end_scope();
    }

    return eval::object::invalid;
}

eval::object::object
Resolver::visit_get_expression(ast::get_expression& get_expression)
{
    get_expression.target->accept(*this);
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_call_expression(ast::call_expression& call_expression)
{
    call_expression.target->accept(*this);

    for (auto& arg : call_expression.args)
    {
        arg->accept(*this);
    }

    return eval::object::invalid;
}

eval::object::object Resolver::visit_function_expression(
    ast::function_expression& function_expression)
{
    begin_scope();
    function_expression.body->accept(*this);
    end_scope();
    return eval::object::invalid;
}

eval::object::object
Resolver::visit_let_expression(ast::let_expression& let_expression)
{
    begin_scope();

    for (auto& binding : let_expression.bindings)
    {
        binding.value->accept(*this);
    }

    let_expression.expr->accept(*this);

    end_scope();
    return eval::object::invalid;
}

eval::object::object Resolver::visit_array(ast::array& array)
{
    for (auto& elem : array.elems)
    {
        elem->accept(*this);
    }

    return eval::object::invalid;
}

eval::object::object Resolver::visit_dictionary(ast::dictionary& dict)
{
    for (auto& key : dict.keys)
        key->accept(*this);
    for (auto& value : dict.values)
        value->accept(*this);
    return eval::object::invalid;
}

eval::object::object Resolver::visit_number(ast::number&)
{
    return eval::object::invalid;
}

eval::object::object Resolver::visit_string(ast::string&)
{
    return eval::object::invalid;
}

eval::object::object Resolver::visit_identifier(ast::identifier& ident)
{
    assign_scope(ident);
    return eval::object::invalid;
}

eval::object::object Resolver::visit_unit(ast::unit&)
{
    return eval::object::invalid;
}

eval::object::object Resolver::visit_placeholder(ast::placeholder&)
{
    return eval::object::invalid;
}

}
