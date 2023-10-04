#pragma once

#include <ast/forward.hpp>
#include <env.hpp>

namespace gaya::ast
{

struct identifier final : public expression
{
    identifier(span s, const std::string& v)
        : _span { s }
        , value { v }
        , key { value }
    {
    }

    object accept(ast_visitor&) override;

    span _span;
    std::string value;
    gaya::eval::key key;
    size_t depth          = 0;
    bool did_assign_scope = false;
};

}
