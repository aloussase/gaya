#pragma once

#include <memory>
#include <vector>

#include <lexer.hpp>
#include <object.hpp>
#include <span.hpp>

namespace ast {

struct ast_node;
struct stmt;
struct expression;
struct identifier;

using node_ptr       = std::unique_ptr<ast_node>;
using stmt_ptr       = std::unique_ptr<stmt>;
using expression_ptr = std::unique_ptr<expression>;

class ast_visitor;

struct ast_node {
  virtual ~ast_node() {};
  virtual std::string to_string() const noexcept  = 0;
  virtual object::object_ptr accept(ast_visitor&) = 0;
};

struct program final : public ast_node {
  std::vector<stmt_ptr> stmts;

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;
};

/* Statements */

struct stmt : public ast_node {
  virtual ~stmt() { }
};

struct declaration_stmt final : public stmt {
  declaration_stmt(std::unique_ptr<identifier> i, expression_ptr e)
      : _identifier { std::move(i) }
      , expression { std::move(e) }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  std::unique_ptr<identifier> _identifier;
  expression_ptr expression;
};

struct expression_stmt final : public stmt {
  expression_stmt(expression_ptr e)
      : expr { std::move(e) }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  expression_ptr expr;
};

/* Expressions */

struct expression : public ast_node {
  virtual ~expression() { }
};

struct call_expression final : public expression {
  call_expression(
      std::unique_ptr<ast::identifier> i, //
      std::vector<expression_ptr>&& a
  )
      : identifier { std::move(i) }
      , args { std::move(a) }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  std::unique_ptr<ast::identifier> identifier;
  std::vector<expression_ptr> args;
};

struct function_expression final : public expression {
  function_expression(std::vector<identifier>&& p, expression_ptr b)
      : params { std::move(p) }
      , body { std::move(b) }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  std::vector<identifier> params;
  expression_ptr body;
};

struct let_expression final : public expression {
  let_expression(
      std::unique_ptr<identifier> i, //
      expression_ptr b,
      expression_ptr e
  )
      : ident { std::move(i) }
      , binding { std::move(b) }
      , expr { std::move(e) }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  std::unique_ptr<identifier> ident;
  expression_ptr binding;
  expression_ptr expr;
};

/* Primary expressions */

struct number final : public expression {
  number(span s, double v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  span _span;
  double value;
};

struct string final : public expression {
  string(span s, const std::string& v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  span _span;
  std::string value;
};

struct identifier final : public expression {
  identifier(span s, const std::string& v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  span _span;
  std::string value;
};

struct unit final : public expression {
  unit(span s)
      : _span { s }
  {
  }

  std::string to_string() const noexcept override;

  object::object_ptr accept(ast_visitor&) override;

  span _span;
};

}
