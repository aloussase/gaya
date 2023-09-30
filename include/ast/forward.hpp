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
};

struct stmt : public ast_node
{
    virtual ~stmt() { }
};

}
