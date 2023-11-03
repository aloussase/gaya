#pragma once

#include <ast/forward.hpp>

namespace gaya::ast
{

struct EnumDeclaration final : stmt
{
    EnumDeclaration(
        span s,
        const std::string& n,
        const std::vector<std::string> v)
        : span_ { s }
        , name { n }
        , variants { v }
    {
    }

    object accept(ast_visitor& v) override;

    span span_;
    std::string name;
    std::vector<std::string> variants;
};

}
