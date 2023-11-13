#pragma once

#include <ast/forward.hpp>

namespace gaya::ast
{

struct Upto final : expression
{
    Upto(span s, expression_ptr start_, expression_ptr end_)
        : span_ { s }
        , start { start_ }
        , end { end_ }
    {
    }

    object accept(ast_visitor& v) override;

    span span_;
    expression_ptr start;
    expression_ptr end;
};

}
