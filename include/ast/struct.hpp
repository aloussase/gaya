#pragma once

#include <ast/forward.hpp>

namespace gaya::ast
{

struct StructField
{
    StructField(const std::string& i, types::Type t)
        : identifier { i }
        , type { t }
    {
    }

    std::string identifier;
    types::Type type;
};

struct StructDeclaration final : stmt
{
    StructDeclaration(span s, const std::string& n, std::vector<StructField> f)
        : span_ { s }
        , name { n }
        , fields { f }
    {
    }

    object accept(ast_visitor& v) override;

    span span_;
    std::string name;
    std::vector<StructField> fields;
};

}
