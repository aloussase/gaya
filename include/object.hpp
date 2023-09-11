#pragma once

#include <memory>
#include <vector>

#include <env.hpp>
#include <span.hpp>

namespace gaya::ast
{
struct identifier;
struct expression;
using expression_ptr = std::unique_ptr<expression>;
}

namespace gaya::eval
{
class interpreter;
}

namespace gaya::eval::object
{

struct object;

using object_ptr = std::shared_ptr<object>;

struct object
{
    virtual ~object() { }
    virtual std::string to_string() const noexcept = 0;
    virtual bool is_callable() const noexcept      = 0;
    virtual std::string typeof_() const noexcept   = 0;
    virtual bool is_truthy() const noexcept        = 0;
};

struct callable : public object
{
    virtual ~callable() { }
    virtual size_t arity() const noexcept = 0;
    virtual object_ptr
    call(interpreter&, span, std::vector<object_ptr>) noexcept
        = 0;

    std::string typeof_() const noexcept override;
    bool is_truthy() const noexcept override;
};

struct function final : public callable
{
    function(
        span s,
        std::vector<ast::identifier> p,
        std::shared_ptr<ast::expression> b,
        env);

    std::string to_string() const noexcept override;
    bool is_callable() const noexcept override;
    size_t arity() const noexcept override;
    object_ptr
    call(interpreter&, span, std::vector<object_ptr> args) noexcept override;

    span _span;
    std::vector<ast::identifier> params;
    size_t _arity;
    std::shared_ptr<ast::expression> body;
    env closed_over_env;
};

struct builtin_function : public callable
{
    builtin_function(const std::string& n)
        : name { n }
    {
    }

    virtual ~builtin_function() { }

    std::string to_string() const noexcept override;
    bool is_callable() const noexcept override;

    std::string name;
};

struct number final : public object
{
    number(span s, double v)
        : _span { s }
        , value { v }
    {
    }

    std::string to_string() const noexcept override;
    bool is_callable() const noexcept override;
    std::string typeof_() const noexcept override;
    bool is_truthy() const noexcept override;

    span _span;
    double value;
};

struct string final : public object
{
    string(span s, const std::string& v)
        : _span { s }
        , value { v }
    {
    }

    std::string to_string() const noexcept override;
    bool is_callable() const noexcept override;
    std::string typeof_() const noexcept override;
    bool is_truthy() const noexcept override;

    span _span;
    std::string value;
};

struct unit final : public object
{
    unit(span s)
        : _span { s }
    {
    }

    std::string to_string() const noexcept override;
    bool is_callable() const noexcept override;
    std::string typeof_() const noexcept override;
    bool is_truthy() const noexcept override;

    span _span;
};

}
