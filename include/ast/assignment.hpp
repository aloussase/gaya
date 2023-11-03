#pragma once

#include <ast/forward.hpp>

namespace gaya::ast
{

enum class AssignmentKind {
    Identifier,
    GetExpression,
    CallExpression,
};

struct assignment_stmt final : public stmt
{
    assignment_stmt(AssignmentKind k, expression_ptr t, expression_ptr e)
        : kind { k }
        , target { std::move(t) }
        , expression { std::move(e) }
    {
    }

    object accept(ast_visitor&) override;

    void replace_child(ast_node* child, std::shared_ptr<ast_node> with) noexcept
        override;

    AssignmentKind kind;
    expression_ptr target;
    expression_ptr expression;
};

}
