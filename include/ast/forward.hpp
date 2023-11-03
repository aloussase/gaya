#pragma once

#include <memory>

#include <object.hpp>

namespace gaya::eval
{
class interpreter;
}

namespace gaya::ast
{

class ast_visitor;

using object = gaya::eval::object::object;

struct ast_node;
struct stmt;
struct expression;
struct identifier;

using node_ptr       = std::shared_ptr<ast_node>;
using stmt_ptr       = std::shared_ptr<stmt>;
using expression_ptr = std::shared_ptr<expression>;

struct ast_node
{
    virtual ~ast_node() {};
    virtual object accept(ast_visitor&) = 0;

    void set_parent(std::shared_ptr<ast_node> new_parent) noexcept
    {
        parent = new_parent;
    }

    virtual void replace_child(
        [[maybe_unused]] ast_node* _child,
        [[maybe_unused]] std::shared_ptr<ast_node> _with) noexcept
    {
    }

    void replace_self_with(std::shared_ptr<ast_node> new_node) noexcept
    {
        if (!parent) return;
        parent->replace_child(this, new_node);
    }

    std::shared_ptr<ast_node> parent = nullptr;
};

struct stmt : public ast_node
{
    virtual ~stmt() { }
};

struct expression : public ast_node
{
    virtual ~expression() { }
};

}
