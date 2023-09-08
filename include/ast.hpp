#pragma once

#include <memory>
#include <vector>

#include <lexer.hpp>
#include <span.hpp>

namespace ast {

struct ast_node;
struct stmt;
struct expression;
struct identifier;

using node_ptr       = std::unique_ptr<ast_node>;
using stmt_ptr       = std::unique_ptr<stmt>;
using expression_ptr = std::unique_ptr<expression>;

struct ast_node {
  virtual ~ast_node() {};

  virtual std::string to_string() const noexcept = 0;
};

struct program final : public ast_node {
  std::vector<stmt_ptr> stmts;

  std::string to_string() const noexcept override;
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

  std::unique_ptr<identifier> _identifier;
  expression_ptr expression;
};

struct expression_stmt final : public stmt {
  expression_stmt(expression_ptr e)
      : expr { std::move(e) }
  {
  }

  std::string to_string() const noexcept override;

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

  std::vector<identifier> params;
  expression_ptr body;
};

/* Primary expressions */

struct number final : public expression {
  number(span s, int v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

  span _span;
  int value;
};

struct string final : public expression {
  string(span s, const std::string& v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

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

  span _span;
  std::string value;
};

struct unit final : public expression {
  unit(span s)
      : _span { s }
  {
  }

  std::string to_string() const noexcept override;

  span _span;
};

}
