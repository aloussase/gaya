#pragma once

#include <exception>
#include <vector>

#include <ast.hpp>
#include <diagnostic.hpp>
#include <lexer.hpp>

class synchronize : public std::exception { };

class parser {
public:
  parser(const char* source);

  [[nodiscard]] ast::node_ptr parse() noexcept;

  [[nodiscard]] std::vector<diagnostic::diagnostic>
  diagnostics() const noexcept;

private:
  std::vector<ast::stmt_ptr> stmts() noexcept;

  [[nodiscard]] ast::stmt_ptr declaration_stmt(token identifier);
  [[nodiscard]] ast::stmt_ptr expression_stmt(token discard);
  [[nodiscard]] ast::expression_ptr expression(token);
  [[nodiscard]] ast::expression_ptr function_expression(token lcurly);
  [[nodiscard]] ast::expression_ptr call_expression(token identifier);
  [[nodiscard]] ast::expression_ptr primary_expression(token);

  [[noreturn]] void parser_error(span, const std::string&);

  const char* _source;
  lexer _lexer;
  std::vector<diagnostic::diagnostic> _diagnostics;
};
