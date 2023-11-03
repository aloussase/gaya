#pragma once

#include <ast/forward.hpp>

namespace gaya::ast
{

struct LessThanNumbers final : public expression
{
    LessThanNumbers(span s, expression_ptr l, expression_ptr r)
        : span_ { s }
        , lhs { l }
        , rhs { r }
    {
    }

    object accept(ast_visitor& v) override;

    span span_;
    expression_ptr lhs;
    expression_ptr rhs;
};

}
