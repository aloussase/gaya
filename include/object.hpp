#pragma once

#include <memory>
#include <vector>

#include <span.hpp>

namespace ast {

struct identifier;
struct expression;

using expression_ptr = std::unique_ptr<expression>;

}

namespace gaya::eval::object {

struct object;

using object_ptr = std::shared_ptr<object>;

struct object {
  virtual ~object() { }
  virtual std::string to_string() const noexcept = 0;
};

struct function final : public object {
  function(span s, std::vector<ast::identifier> p, ast::expression_ptr b);

  std::string to_string() const noexcept override;

  span _span;
  std::vector<ast::identifier> params;
  size_t arity;
  ast::expression_ptr body;
};

struct number final : public object {
  number(span s, double v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

  span _span;
  double value;
};

struct string final : public object {
  string(span s, const std::string& v)
      : _span { s }
      , value { v }
  {
  }

  std::string to_string() const noexcept override;

  span _span;
  std::string value;
};

struct unit final : public object {
  unit(span s)
      : _span { s }
  {
  }

  std::string to_string() const noexcept override;

  span _span;
};

}
